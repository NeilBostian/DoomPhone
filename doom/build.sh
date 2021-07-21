#!/bin/bash

# Exit on any errors
set -e

# Source  our shell extensions
. ../shell-extensions.sh

make

# Make sure the destination copy directory exists,
# remove any pre-existing files
ssh_exec "rm -f /usr/local/doom/doom"

# Copy our new executable
ssh_cp bin/doom /usr/local/doom/doom
