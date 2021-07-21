#!/bin/bash

# Exit on any errors
set -e

# Source  our shell extensions
. ../shell-extensions.sh

# Compile our exe to ./bin directory
mkdir -p bin
arm-linux-gnueabi-gcc keypad-driver.c -o bin/keypad-driver

# Copy our exe and run it
ssh_cp bin/keypad-driver /keypad-driver
ssh_exec "chmod +x /keypad-driver && cat /dev/input/event0 | /keypad-driver"

