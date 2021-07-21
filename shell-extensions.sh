SSH_USER=root@192.168.1.4
SSH_PASS=456
SSH_PORT=2222

# Executes shell command on phone over ssh
# Usage:
# user@ubuntu:~/repos/doomphone$ ssh_exec "echo 'Hello, from polycom'"
ssh_exec() {
	sshpass -p $SSH_PASS ssh -tt $SSH_USER -p $SSH_PORT "$1"
}

# Copies file from host machine to phone
# scp didn't work for me, so this is implemented using pipes. seems to work ok.
# Usage:
# user@ubuntu:~/repos/doomphone$ ssh_cp host_file.txt /remote_file.txt
ssh_cp() {
	cat $1 | sshpass -p $SSH_PASS ssh $SSH_USER -p $SSH_PORT "cat > $2"
}

# Takes a (raw) screenshot of the framebuffer device
# Usage:
# user@ubuntu:~/repos/doomphone$ take_screenshot save-file.raw
take_screenshot() {
    sshpass -p $SSH_PASS ssh $SSH_USER -p $SSH_PORT "cat /dev/fb0" > "$1"
}
