CC=gcc
FLAGS=-Wall -Wextra -std=c11 -fPIC
MODULES_C=$(shell find . -maxdepth 1 -name "*.c")
MODULES_O=$(MODULES_C:.c=.o)
MODULES_SO=$(MODULES_C:.c=.so)

.PHONY: all
all: clean
	$(MAKE) lib

.PHONY: lib
lib: $(MODULES_O) $(MODULES_SO)
	$(MAKE) clean_o

%.o: %.c
	$(CC) $(FLAGS) -c $< -o $@

%.so: %.o
	$(CC) -shared $< -o $@

.PHONY: fmt
fmt:
	clang-format -i $(MODULES_C)

.PHONY: clean
clean: clean_o clean_lib

.PHONY: clean_o
clean_o:
	rm -rf *.o

.PHONY: clean_lib
clean_lib:
	rm -rf *.so