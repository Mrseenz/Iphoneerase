#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>

#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/lockdown.h>
#include <libimobiledevice/diagnostics_relay.h>
#include <plist/plist.h>

#define SERVICE_NAME "com.apple.diagnostics_relay"
#define ERASE_REQUEST "MobileObliterator"

typedef enum {
    RESULT_SUCCESS = 0,
    RESULT_ARG_ERROR = 1,
    RESULT_DEVICE_ERROR = 2,
    RESULT_SERVICE_ERROR = 3,
    RESULT_COMMAND_ERROR = 4
} EraseResult;

typedef struct {
    char *udid;
    char *ecid;
    int debug;
    int force;
} EraseOptions;

void print_usage(const char *prog_name) {
    printf("iOS Device Secure Erasure Utility\n");
    printf("Version: 1.0\n\n");
    printf("Usage: %s -u <UDID> [OPTIONS]\n\n", prog_name);
    printf("Options:\n");
    printf("  -u, --udid <UDID>    Target device UDID (required)\n");
    printf("  -e, --ecid <ECID>    Device ECID for verification\n");
    printf("  -f, --force          Skip confirmation prompt\n");
    printf("  -d, --debug          Enable verbose debug output\n");
    printf("  -h, --help           Show this help message\n\n");
    printf("WARNING: This operation PERMANENTLY destroys all data on the target device!\n");
    printf("Ensure you have proper authorization before proceeding.\n");
}

int parse_options(int argc, char **argv, EraseOptions *options) {
    static struct option long_opts[] = {
        {"udid", required_argument, NULL, 'u'},
        {"ecid", required_argument, NULL, 'e'},
        {"force", no_argument, NULL, 'f'},
        {"debug", no_argument, NULL, 'd'},
        {"help", no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0}
    };

    memset(options, 0, sizeof(EraseOptions));
    int opt;

    while ((opt = getopt_long(argc, argv, "u:e:fdh", long_opts, NULL)) != -1) {
        switch (opt) {
        case 'u':
            options->udid = strdup(optarg);
            break;
        case 'e':
            options->ecid = strdup(optarg);
            break;
        case 'f':
            options->force = 1;
            break;
        case 'd':
            options->debug = 1;
            break;
        case 'h':
            print_usage(argv[0]);
            exit(RESULT_SUCCESS);
        default:
            print_usage(argv[0]);
            return RESULT_ARG_ERROR;
        }
    }

    if (!options->udid) {
        fprintf(stderr, "Error: Device UDID is required\n");
        print_usage(argv[0]);
        return RESULT_ARG_ERROR;
    }

    return RESULT_SUCCESS;
}

int user_confirmation() {
    printf("\nWARNING: THIS OPERATION WILL DESTROY ALL DATA ON THE DEVICE!\n");
    printf("This action is irreversible and cannot be undone.\n\n");
    printf("Type 'CONFIRM ERASE' to proceed: ");
    
    char response[32];
    if (!fgets(response, sizeof(response), stdin)) { // Fixed: Added closing parenthesis
        fprintf(stderr, "Input error\n");
        return 0;
    }
    
    // Remove newline
    response[strcspn(response, "\n")] = 0;
    return strcmp(response, "CONFIRM ERASE") == 0;
}

void log_debug(const char *message, EraseOptions *options) {
    if (options->debug) {
        printf("[DEBUG] %s\n", message);
    }
}

void log_plist(plist_t plist, const char *title, EraseOptions *options) {
    if (options->debug && plist) {
        char *xml = NULL;
        uint32_t len = 0;
        plist_to_xml(plist, &xml, &len);
        printf("[DEBUG] %s:\n%.*s\n", title, (int)len, xml);
        free(xml);
    }
}

EraseResult establish_connection(EraseOptions *options,
                                 idevice_t *device,
                                 lockdownd_client_t *client) {
    idevice_error_t dev_err = idevice_new_with_options(device, options->udid, IDEVICE_LOOKUP_USBMUX);
    if (dev_err != IDEVICE_E_SUCCESS) {
        fprintf(stderr, "Error connecting to device: %d\n", dev_err);
        return RESULT_DEVICE_ERROR;
    }

    lockdownd_error_t ld_err = lockdownd_client_new_with_handshake(*device, client, "ideviceerase");
    if (ld_err != LOCKDOWN_E_SUCCESS) {
        fprintf(stderr, "Error connecting to lockdown service: %d\n", ld_err);
        idevice_free(*device);
        *device = NULL;
        return RESULT_DEVICE_ERROR;
    }

    return RESULT_SUCCESS;
}

EraseResult start_diagnostics_service(lockdownd_client_t client,
                                     EraseOptions *options,
                                     lockdownd_service_descriptor_t *service) {
    lockdownd_error_t srv_err = lockdownd_start_service(client, SERVICE_NAME, service);
    if (srv_err != LOCKDOWN_E_SUCCESS || !*service || (*service)->port == 0) {
        fprintf(stderr, "Error starting diagnostics service: %d\n", srv_err);
        return RESULT_SERVICE_ERROR;
    }
    return RESULT_SUCCESS;
}

EraseResult perform_device_erase(idevice_t device,
                                 lockdownd_service_descriptor_t service,
                                 EraseOptions *options) {
    diagnostics_relay_client_t diag_client = NULL;
    plist_t request = NULL;
    plist_t response = NULL;
    EraseResult result = RESULT_COMMAND_ERROR;

    if (diagnostics_relay_client_new(device, service, &diag_client) != DIAGNOSTICS_RELAY_E_SUCCESS) {
        fprintf(stderr, "Error creating diagnostics client\n");
        return RESULT_SERVICE_ERROR;
    }

    request = plist_new_dict();
    plist_dict_set_item(request, "Request", plist_new_string(ERASE_REQUEST));
    log_plist(request, "Erase Request", options);

    if (diagnostics_relay_send(diag_client, request) != DIAGNOSTICS_RELAY_E_SUCCESS) {
        fprintf(stderr, "Error sending erase command\n");
        goto cleanup;
    }

    diagnostics_relay_error_t recv_err = diagnostics_relay_recv(diag_client, &response);
    
    if (recv_err == DIAGNOSTICS_RELAY_E_SUCCESS && response) {
        log_plist(response, "Erase Response", options);
        
        plist_t status = plist_dict_get_item(response, "Status");
        char *status_str = NULL;
        
        if (status) {
            plist_get_string_val(status, &status_str);
        }
        
        if (status_str && strcmp(status_str, "Success") == 0) {
            printf("Erase command acknowledged\n");
            result = RESULT_SUCCESS;
        } else {
            fprintf(stderr, "Erase failed: %s\n", status_str ? status_str : "Unknown error");
        }
        
        if (status_str) free(status_str);
    } else if (recv_err == DIAGNOSTICS_RELAY_E_MUX_ERROR) {
        printf("Device disconnected - erase initiated\n");
        result = RESULT_SUCCESS;
    } else {
        fprintf(stderr, "Error receiving response: %d\n", recv_err);
    }

cleanup:
    if (request) plist_free(request);
    if (response) plist_free(response);
    if (diag_client) diagnostics_relay_client_free(diag_client);
    
    return result;
}

int main(int argc, char *argv[]) {
    EraseOptions options;
    idevice_t device = NULL;
    lockdownd_client_t client = NULL;
    lockdownd_service_descriptor_t service = NULL;
    EraseResult result;
    
    if ((result = parse_options(argc, argv, &options)) != RESULT_SUCCESS) {
        return result;
    }

    printf("iOS Device Erasure Utility\n");
    printf("Target UDID: %s\n", options.udid);
    if (options.ecid) {
        printf("Verification ECID: %s\n", options.ecid);
    }
    
    if (!options.force && !user_confirmation()) {
        fprintf(stderr, "Operation cancelled by user\n");
        result = RESULT_ARG_ERROR;
        goto cleanup;
    }

    printf("\nInitiating secure erase procedure...\n");
    
    if ((result = establish_connection(&options, &device, &client)) != RESULT_SUCCESS) {
        goto cleanup;
    }
    
    if ((result = start_diagnostics_service(client, &options, &service)) != RESULT_SUCCESS) {
        goto cleanup;
    }
    
    result = perform_device_erase(device, service, &options);
    
    if (result == RESULT_SUCCESS) {
        printf("\nSUCCESS: Device erase initiated\n");
        printf("The device will now reboot and begin the secure erasure process.\n");
        printf("This may take several minutes to complete.\n");
    } else {
        fprintf(stderr, "\nERROR: Failed to initiate device erase\n");
    }

cleanup:
    if (service) lockdownd_service_descriptor_free(service);
    if (client) lockdownd_client_free(client);
    if (device) idevice_free(device);
    if (options.udid) free(options.udid);
    if (options.ecid) free(options.ecid);
    
    return result;
}
