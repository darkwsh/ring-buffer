obj-m = tmain.o

tmain-y = main.o src/ring_buffer.o

all:
	make -C /lib/modules/`uname -r`/build EXTRA_CFLAGS=-I$(shell pwd)/include  M=`pwd`

clean:
	rm -rf *.o *.ko *.mod.c modules.order Module.symvers *~ 
	rm -rf include/*~ include/*.o src/*~ src/*.o 
