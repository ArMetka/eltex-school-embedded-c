obj-m += khello.o

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

.PHONY: clean-clang
clean-clang:
	rm -f compile_commands.json

.PHONY: insmod
insmod:
	insmod khello.ko

.PHONY: rmmod
rmmod:
	rmmod khello

.PHONY: lsmod
lsmod:
	lsmod | grep khello -A 3 -B 3 || true

.PHONY: dmesg
dmesg:
	dmesg | tail -n 1