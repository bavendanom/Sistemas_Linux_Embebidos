# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -D_POSIX_C_SOURCE=200809L
LDFLAGS = -lrt

# Directories
SRC_DIR = src
BUILD_DIR = build
SYSTEMD_DIR = systemd
TESTS_DIR = tests

# Targets
TARGET = $(BUILD_DIR)/assignment-sensor
SRC = $(SRC_DIR)/main.c

# Default target
all: $(TARGET)

# Create build directory and compile
$(TARGET): $(SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Install to system
install: $(TARGET)
	@echo "Installing binary to /usr/local/bin/"
	sudo cp $(TARGET) /usr/local/bin/
	@echo "Installing systemd service file"
	sudo cp $(SYSTEMD_DIR)/assignment-sensor.service /etc/systemd/system/
	sudo systemctl daemon-reload

# Uninstall
uninstall:
	@echo "Removing installed files"
	sudo rm -f /usr/local/bin/assignment-sensor
	sudo rm -f /etc/systemd/system/assignment-sensor.service
	sudo systemctl daemon-reload

# Clean build files
clean:
	rm -rf $(BUILD_DIR)

# Run tests
test: $(TARGET)
	@echo "Running smoke tests..."
	$(TESTS_DIR)/smoke-test.sh

# Enable and start service
enable-service: install
	sudo systemctl enable assignment-sensor.service
	sudo systemctl start assignment-sensor.service
	@echo "Service enabled and started"

# Disable and stop service
disable-service:
	sudo systemctl stop assignment-sensor.service
	sudo systemctl disable assignment-sensor.service
	@echo "Service stopped and disabled"

.PHONY: all install uninstall clean test enable-service disable-service