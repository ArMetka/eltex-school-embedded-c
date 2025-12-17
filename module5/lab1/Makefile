obj-m += k-hello.o

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
