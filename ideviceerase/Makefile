CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = $(shell pkg-config --libs libimobiledevice-1.0)
INCLUDES = $(shell pkg-config --cflags libimobiledevice-1.0)

TARGET = ideviceerase
SRC = src/ideviceerase.c

.PHONY: all clean install uninstall

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TARGET)

install: $(TARGET)
	cp $(TARGET) /usr/local/bin/
	chmod 755 /usr/local/bin/$(TARGET)

uninstall:
	rm -f /usr/local/bin/$(TARGET)
