obj-m := testDriver.o
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

limpa:
	rm -f *.o *.ko *.symvers *.order *.mod*
carrega:
	sudo insmod testDriver.ko
no:
	sudo mknod -m 666 /dev/DriverSOi2c c 243 0
	
descarrega:
	sudo rmmod testDriver.ko
	sudo rm -f /dev/DriverSOi2c
run:
	gcc -o main main.c userLevelFunctions.c
	./main
deleteApp:
	rm -f main
