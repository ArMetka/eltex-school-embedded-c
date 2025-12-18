MODULE_NAME=procio
obj-m += $(MODULE_NAME).o

all:
	$(MAKE) modules

.PHONY: modules
modules:
	mv ./compile_commands.json ./.compile_commands.json 2>/dev/null || true
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	mv ./.compile_commands.json ./compile_commands.json 2>/dev/null || true

.PHONY: clean
clean:
	mv ./compile_commands.json ./.compile_commands.json 2>/dev/null || true
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	mv ./.compile_commands.json ./compile_commands.json 2>/dev/null || true

.PHONY: clang
clang:
	rm -f compile_commands.json
	bear -- $(MAKE) modules
	$(MAKE) clean

.PHONY: clean-clang
clean-clang:
	rm -f compile_commands.json

.PHONY: insmod
insmod:
	insmod $(MODULE_NAME).ko

.PHONY: rmmod
rmmod:
	rmmod $(MODULE_NAME)

.PHONY: lsmod
lsmod:
	lsmod | grep $(MODULE_NAME) -A 3 -B 3 || true

.PHONY: dmesg
dmesg:
	dmesg | tail -n 1