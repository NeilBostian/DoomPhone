#!/bin/bash
# Execute from host PC to install phone_utils on the target phone

# Source  our shell extensions
. ../shell-extensions.sh

ssh_cp phone_utils.sh /usr/local/phone_utils.sh
ssh_cp pause_phone.sh /usr/local/pause_phone.sh
ssh_cp resume_phone.sh /usr/local/resume_phone.sh

ssh_exec "chmod +x /usr/local/pause_phone.sh && chmod +x /usr/local/resume_phone.sh"
