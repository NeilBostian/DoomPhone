#!/bin/sh

# run_doom.sh script runs on polycom phone
# from /usr/local/doom/run_doom.sh
# This script pauses the phone application
# and starts doom. You must run install.sh
# and build.sh from host machine before this
# script can be run from the phone (via ssh
# or other method of executing scripts)

# Exit on any errors
set -e

# Pause any running phone applications
cd /usr/local
./pause_phone.sh

# Start doom executable
cd /usr/local/doom
chmod +x ./doom
cat /dev/input/event0 | ./doom -config doomconfig.cfg -extraconfig extradoomconfig.cfg
