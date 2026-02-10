# Build investigation kernel modules
# Usage: make -C /lib/modules/$(uname -r)/build M=$(pwd) modules

obj-m += nhi_regdump.o
obj-m += read_p1wg3.o
obj-m += read_pcha_mod.o

all:
	make -C /lib/modules/$$(uname -r)/build M=$$(pwd) modules

clean:
	make -C /lib/modules/$$(uname -r)/build M=$$(pwd) clean
