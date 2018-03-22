#!/bin/sh

case $1 in
	start)
            rm -rf /dev/r2l
		modprobe r2l
		mknod /dev/r2l c 777 0
		chmod 777 /dev/r2l
		;;
	stop)
		;;
	reload)
		;;
	restart|force-reload)
		rm -rf /dev/r2l
		modprobe r2l
		mknod /dev/r2l c 777 0
		chmod 777 /dev/r2l
		;;
        status)
		;;
	*)
		echo "Usage: /etc/init.d/r2l {start|stop|reload|restart|force-reload|status}"
		exit 1
		;;
esac

exit 0
