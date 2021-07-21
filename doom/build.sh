#!/bin/bash

# Exit on any errors
set -e

# Source  our shell extensions
. ../shell-extensions.sh

make

# Remove pre-existing binary
ssh_exec "rm -f /usr/local/doom/doom"

# Copy our new executable
ssh_cp bin/doom /usr/local/doom/doom
