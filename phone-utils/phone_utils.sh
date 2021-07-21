# get PID for process with commandline $1
get_pid() {
	REPLVAR=$(echo "$1" | sed 's#/#\\/#g')
	PID=$(ps -o pid,args -C bash | awk "/$REPLVAR/"' { print $1 }')
	echo $PID
}

# Find a process based on the command line and stop it
stop_proc() {
	PID=$(get_pid "$1")

	if [ ! -z "$PID" ]
	then
		echo "Stopped '$1', PID=$PID"
		kill -STOP $PID
	fi
}

# Find a process based on the command line and resume it
start_proc() {
	PID=$(get_pid "$1")

	if [ ! -z "$PID" ]
	then
		echo "Resumed '$1', PID=$PID"
		kill -CONT $PID
	fi
}

stop_procs() {
	stop_proc "/bin/sh /etc/init.d/rcS"
	stop_proc "/bin/sh /etc/rcS.d/S45polyLauncher"
	stop_proc "/usr/bin/polylauncher"
	stop_proc "/bin/sh /usr/local/etc/init.d/rcS"
	stop_proc "/usr/local/bin/plcmWatchdog"
	stop_proc "/usr/local/bin/dbus-daemon --system"
	stop_proc "/usr/local/bin/dbus-daemon --session"
	stop_proc "/usr/local/bin/dbus-daemon --fork --print-pid 4 --pri"
	stop_proc "/bin/sh /usr/local/etc/init.d/rcS"
	stop_proc "/bin/sh /usr/local/etc/rcS.d/S90spipguicore"
	stop_proc "/usr/local/bin/os2app listen"
	stop_proc "/bin/sh ./startSpipGUI.sh"
	stop_proc "/bin/sh ./startSpipGUIProfile.sh"
	stop_proc "./spipgui -qws -display dbscreen:1 -nomouse"
	stop_proc "./browser -display :1"
	stop_proc "./ec"
	stop_proc "./em"
	stop_proc "./dbs"
	stop_proc "./osd"
}

start_procs() {
	start_proc "/bin/sh /etc/init.d/rcS"
	start_proc "/bin/sh /etc/rcS.d/S45polyLauncher"
	start_proc "/usr/bin/polylauncher"
	start_proc "/bin/sh /usr/local/etc/init.d/rcS"
	start_proc "/usr/local/bin/plcmWatchdog"
	start_proc "/usr/local/bin/dbus-daemon --system"
	start_proc "/usr/local/bin/dbus-daemon --session"
	start_proc "/usr/local/bin/dbus-daemon --fork --print-pid 4 --pri"
	start_proc "/bin/sh /usr/local/etc/init.d/rcS"
	start_proc "/bin/sh /usr/local/etc/rcS.d/S90spipguicore"
	start_proc "/usr/local/bin/os2app listen"
	start_proc "/bin/sh ./startSpipGUI.sh"
	start_proc "/bin/sh ./startSpipGUIProfile.sh"
	start_proc "./spipgui -qws -display dbscreen:1 -nomouse"
	start_proc "./browser -display :1"
	start_proc "./ec"
	start_proc "./em"
	start_proc "./dbs"
	start_proc "./osd"
}
