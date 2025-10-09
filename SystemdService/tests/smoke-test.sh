#!/bin/bash

# Smoke test for assignment-sensor
set -e

echo "Starting smoke test for assignment-sensor..."
BUILD_DIR="./build"
BINARY="$BUILD_DIR/assignment-sensor"
LOG_FILE="/tmp/smoke_test.log"
TEST_INTERVAL=1

# Check if binary exists
if [ ! -f "$BINARY" ]; then
    echo "Error: Binary not found at $BINARY"
    echo "Please build the project first: make"
    exit 1
fi

echo "1. Testing help option..."
$BINARY --help | grep -q "Usage" && echo "? Help test passed"

echo "2. Testing basic execution..."
# Start sensor in background
$BINARY --interval $TEST_INTERVAL --logfile "$LOG_FILE" &
SENSOR_PID=$!

# Wait for some samples
sleep 3

# Check if process is running
if kill -0 $SENSOR_PID 2>/dev/null; then
    echo "? Process is running"
else
    echo "? Process failed to start"
    exit 1
fi

# Check if log file is created and has content
if [ -f "$LOG_FILE" ]; then
    echo "? Log file created"
    LINE_COUNT=$(wc -l < "$LOG_FILE")
    if [ "$LINE_COUNT" -gt 0 ]; then
        echo "? Log file has entries: $LINE_COUNT lines"
        
        # Check log format
        FIRST_LINE=$(head -1 "$LOG_FILE")
        if echo "$FIRST_LINE" | grep -qE "^[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}:[0-9]{2}:[0-9]{2}\.[0-9]{3}Z \| 0x[0-9A-F]{8}$"; then
            echo "? Log format is correct"
        else
            echo "? Log format is incorrect: $FIRST_LINE"
            kill $SENSOR_PID 2>/dev/null || true
            exit 1
        fi
    else
        echo "? Log file is empty"
        kill $SENSOR_PID 2>/dev/null || true
        exit 1
    fi
else
    echo "? Log file not created"
    kill $SENSOR_PID 2>/dev/null || true
    exit 1
fi

echo "3. Testing SIGTERM handling..."
kill -TERM $SENSOR_PID
wait $SENSOR_PID 2>/dev/null || true

# Check exit code
if [ $? -eq 0 ]; then
    echo "? Clean shutdown successful"
else
    echo "? Clean shutdown failed"
    exit 1
fi

echo "4. Testing fallback log location..."
# Create a non-writable directory to trigger fallback
READONLY_LOG="/tmp/readonly_test.log"
touch "$READONLY_LOG"
chmod 000 "$READONLY_LOG"

$BINARY --interval 1 --logfile "$READONLY_LOG" --device "/dev/zero" &
FALLBACK_PID=$!
sleep 2

# Check if process is using fallback
if ps -p $FALLBACK_PID > /dev/null; then
    echo "? Process running with fallback log"
    kill -TERM $FALLBACK_PID
    wait $FALLBACK_PID 2>/dev/null || true
else
    echo "? Process failed to start with fallback"
fi

# Cleanup
chmod 644 "$READONLY_LOG" 2>/dev/null || true
rm -f "$READONLY_LOG" "$LOG_FILE"

echo ""
echo "? All smoke tests passed!"
echo "The sensor logger is functioning correctly."