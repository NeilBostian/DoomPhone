#!/bin/bash

# Exit on any errors
set -e

# Source  our shell extensions
. ../shell-extensions.sh

# Compile our exe to ./bin directory
mkdir -p bin
arm-linux-gnueabi-gcc display-driver.c -o bin/display-driver

# Copy our exe and run it
ssh_cp bin/display-driver /display-driver
ssh_exec "chmod +x /display-driver && /display-driver"

