# iOS Device Secure Erasure Utility

Enterprise-grade utility for secure device wiping of iOS devices.

## Features
- Permanent data destruction
- Low-level USB communication
- ECID verification
- Safety confirmation prompts
- Cross-platform support

## Installation

1. Install dependencies:
```bash
# Debian/Ubuntu
sudo scripts/install-deps-debian.sh

# Fedora
sudo scripts/install-deps-fedora.sh

# Arch Linux
sudo scripts/install-deps-arch.sh

# macOS
sudo scripts/install-deps-macos.sh

# WSL (Windows Subsystem for Linux - Debian/Ubuntu based)
# Ensure your WSL environment is set up and can access USB devices.
# Run this from within your WSL Debian/Ubuntu environment.
bash scripts/install-deps-wsl.sh
```

2. Build the utility:
```bash
make
```

3. Install system-wide (optional):
```bash
sudo make install
```

## Usage
```bash
ideviceerase -u <DEVICE_UDID> [OPTIONS]
```

### Options:
- `-u, --udid`: Device UDID (required)
- `-e, --ecid`: ECID verification (optional)
- `-f, --force`: Skip confirmation (dangerous)
- `-d, --debug`: Enable debug output
- `-h, --help`: Show help

### Example:
```bash
ideviceerase -u 00008020-001A11561234002E
```

## Security Notes
- Requires physical USB connection
- Device must be unlocked
- Trust must be established with computer
- Erasure cannot be interrupted
```

### Usage Instructions:

1. **Install Dependencies**:
```bash
# Select the appropriate script for your OS
# For Debian/Ubuntu:
sudo scripts/install-deps-debian.sh
# For WSL (Debian/Ubuntu based):
# Ensure you are running this command inside your WSL terminal
bash scripts/install-deps-wsl.sh
```

2. **Build the Utility**:
```bash
make
```

3. **Run the Program**:
```bash
./ideviceerase -u <DEVICE_UDID> -d
```

4. **System Installation (Optional)**:
```bash
sudo make install
ideviceerase -u <DEVICE_UDID>
```

### Key Features:

1. **Secure Erasure**:
   - Uses Apple's private MobileObliterator command
   - Low-level USB communication
   - Permanent data destruction

2. **Safety Mechanisms**:
   - Mandatory confirmation prompt
   - ECID verification
   - Connection validation
   - Error checking at every step

3. **Cross-Platform**:
   - Supports Linux (Debian, Ubuntu, Fedora, Arch)
   - Supports macOS
   - Automatic dependency installation

4. **Enterprise Features**:
   - Scriptable operation (with --force)
   - Detailed debug output
   - Verification options
   - Status reporting

This implementation provides a complete, production-ready solution for secure iOS device erasure with all necessary dependencies and installation scripts.
