# OSI Lab 8 - Arduino Temperature Sensor Driver

This lab builds on Lab 5 skills. You will:
1. Write a userspace program to communicate with an Arduino over USB serial (sanity test)
2. Extend a character driver to read temperature from a TMP36 sensor via Arduino

## Repository Structure
```text
osi_lab8/
├── README.md
├── part1_sanity_test/
│   ├── arduino_sketch/
│   │   └── tmp36_reader.ino
│   ├── temp_test.c
│   └── Makefile
├── part2_kernel_driver/
│   ├── memory.c
│   ├── Makefile
│   └── test_driver.sh
└── solutions/
    ├── temp_test_complete.c
    └── memory_complete.c

text

- **`part1_sanity_test/`** - Userspace test program and Arduino sketch
- **`part2_kernel_driver/`** - Kernel driver template and Makefile  
- **`solutions/`** - Complete solutions (instructor access only)

## Hardware Required

- Arduino Uno
- TMP36 temperature sensor
- Jumper wires (3)
- USB cable (Type A to B)

## Wiring

Wiring for TMP036 - see https://sl.bing.net/i6FidWrUjwO
- Left (Vcc) connects to 5V
- Middle (Signal) connects to A0
- Right (GND) connects to GND

## Quick Start

See the lab document for full instructions. The main steps are:

```bash
# Part 1 - Sanity test
cd ~/osi_lab8/part1_sanity_test
make
sudo chmod 666 /dev/ttyACM0
./temp_test

# Part 2 - Kernel driver
cd ~/osi_lab8/part2_kernel_driver
make
sudo insmod memory.ko
sudo chmod 666 /dev/memory
cat /dev/memory
sudo rmmod memory


