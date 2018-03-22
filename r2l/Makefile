obj-m := r2l.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=`pwd` V=1 modules
	g++ -o test_r2l test_r2l.cpp -lpthread

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -rf ./test_r2l

try:
	rmmod r2l.ko
	rm -rf /dev/r2l 
	mknod /dev/r2l c 777 0
	chmod 777 /dev/r2l
	insmod r2l.ko

install:
	cp r2l.ko /lib/modules/$(shell uname -r)/
	cd /lib/modules/$(shell uname -r)/
	depmod
	rm -rf /etc/init.d/r2l
	rm -rf /etc/rc0.d/K50r2l
	rm -rf /etc/rc1.d/K50r2l
	rm -rf /etc/rc2.d/S50r2l
	rm -rf /etc/rc3.d/S50r2l
	rm -rf /etc/rc4.d/S50r2l
	rm -rf /etc/rc5.d/S50r2l
	rm -rf /etc/rc6.d/K50r2l

	cp r2l.sh /etc/init.d/r2l
	chown root /etc/init.d/r2l
	chgrp root /etc/init.d/r2l
	chmod 755 /etc/init.d/r2l

	cd /etc/rc0.d/; ln -s ../init.d/r2l K50r2l
	cd /etc/rc1.d/; ln -s ../init.d/r2l K50r2l
	cd /etc/rc2.d/; ln -s ../init.d/r2l S50r2l
	cd /etc/rc3.d/; ln -s ../init.d/r2l S50r2l
	cd /etc/rc4.d/; ln -s ../init.d/r2l S50r2l
	cd /etc/rc5.d/; ln -s ../init.d/r2l S50r2l
	cd /etc/rc6.d/; ln -s ../init.d/r2l K50r2l

uninstall:
	rm -rf /lib/modules/$(shell uname -r)/r2l.ko
	cd /lib/modules/$(shell uname -r)/
	depmod
	rm -rf /etc/init.d/r2l
	rm -rf /etc/rc0.d/K50r2l
	rm -rf /etc/rc1.d/K50r2l
	rm -rf /etc/rc2.d/S50r2l
	rm -rf /etc/rc3.d/S50r2l
	rm -rf /etc/rc4.d/S50r2l
	rm -rf /etc/rc5.d/S50r2l
	rm -rf /etc/rc6.d/K50r2l

