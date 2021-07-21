## doomphone/doom
This folder contains the doom source code, modified from the [doomgeneric](https://github.com/ozkl/doomgeneric/tree/2d9b24f07c78c36becf41d89db30fa99863463e5) repo. The majority of changes from forked source code is contained in [this commit (7895bab)](https://github.com/NeilBostian/DoomPhone/commit/7895babeaadb02c95b00f872c7e034bfc84dd20e).

### Install Doom:
1. Doom dependency files can be installed using script [`./install.sh`](./install.sh)
2. Doom binary can be built and copied to phone using script [`./build.sh`](./build.sh)

### Run Doom:
After installing DOOM, you can run it over ssh:

    # Include shell_extensions in our shell session
    . ../shell_extensions.sh

    # Run doom launcher script
    ssh_exec "/usr/local/doom/run_doom.sh"
