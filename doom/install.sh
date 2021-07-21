#!/bin/bash

# Exit on any errors
set -e

# Source  our shell extensions
. ../shell-extensions.sh

# Create doom directory and copy dependencies to it
ssh_exec "mkdir -p /usr/local/doom"
ssh_cp DOOM1.WAD /usr/local/doom/DOOM1.WAD
ssh_cp run_doom.sh /usr/local/doom/run_doom.sh
ssh_exec "chmod +x /usr/local/doom/run_doom.sh"