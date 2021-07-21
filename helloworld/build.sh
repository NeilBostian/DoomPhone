#!/bin/bash

# Exit on any errors
set -e

# Source  our shell extensions
. ../shell-extensions.sh

# Compile our exe to ./bin directory
mkdir -p bin
arm-linux-gnueabi-gcc helloworld.c -o bin/helloworld

# Copy our exe and run it
ssh_cp bin/helloworld /helloworld
ssh_exec "chmod +x /helloworld && /helloworld"

