#!/bin/bash
# Test script for memory driver (Arduino temperature)
# ATU Sligo - Operating Systems Interfacing Lab 8

echo "=========================================="
echo "Testing Temperature Driver"
echo "=========================================="

# Check Arduino is connected
if [ ! -c /dev/ttyACM0 ]; then
    echo "ERROR: /dev/ttyACM0 not found"
    echo "Connect Arduino and check: ls -l /dev/ttyACM*"
    exit 1
fi

# Set permissions on serial port
sudo chmod 666 /dev/ttyACM0

# Remove old driver if loaded
sudo rmmod memory 2>/dev/null

# Load driver
sudo insmod memory.ko
if [ $? -ne 0 ]; then
    echo "ERROR: Failed to load driver"
    echo "Check dmesg | tail -20"
    exit 1
fi

echo "Driver loaded successfully"

# Check device file was created automatically
if [ -c /dev/memory ]; then
    echo "SUCCESS: /dev/memory created automatically"
    ls -l /dev/memory
else
    echo "ERROR: /dev/memory not found"
    sudo rmmod memory
    exit 1
fi

# Set permissions on device file
sudo chmod 666 /dev/memory

echo ""
echo "Reading temperature (this may take a moment)..."
echo ""

# Read temperature
TEMP=$(cat /dev/memory 2>/dev/null)

if [ -n "$TEMP" ]; then
    echo "Temperature: $TEMP°C"
    echo "SUCCESS: Driver working correctly"
else
    echo "ERROR: No data read from /dev/memory"
    echo "Check Arduino wiring and sketch"
fi

echo ""
echo "Unloading driver..."
sudo rmmod memory

if [ ! -c /dev/memory ]; then
    echo "SUCCESS: /dev/memory removed automatically"
fi

echo "=========================================="
echo "Test complete"
echo "=========================================="