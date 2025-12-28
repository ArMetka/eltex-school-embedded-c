CC=gcc
FLAGS=-Wall -Wextra
SNIFFER_MODULES_C=sniffer.c
SNIFFER_MODULES_O=$(SNIFFER_MODULES_C:.c=.o)
SNIFFER_EXE=sniffer

all: clean
	$(MAKE) sniffer

.PHONY: sniffer
sniffer: $(SNIFFER_MODULES_O)
	$(CC) -o $(SNIFFER_EXE) $(SNIFFER_MODULES_O)
	$(MAKE) clean_o

%.o: %.c
	$(CC) $(FLAGS) -c $< -o $@

.PHONY: fmt
fmt:
	clang-format -i $(SNIFFER_MODULES_C)

.PHONY: clean
clean: clean_o clean_exe

.PHONY: clean_o
clean_o:
	rm -rf *.o

.PHONY: clean_exe
clean_exe:
	rm -rf $(SNIFFER_EXE)
