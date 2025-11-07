CC=gcc
FLAGS=-Wall -Wextra -std=c11
LFLAGS=-ldl
MODULES_C=calc_cli.c
MODULES_O=$(MODULES_C:.c=.o)
SOLUTION_EXE=calc_cli
LIB_DIR=./lib

.PHONY: all
all: clean
	$(MAKE) lib
	$(MAKE) solution

.PHONY: lib
lib:
	$(MAKE) -C $(LIB_DIR)

.PHONY: solution
solution: $(MODULES_O)
	$(CC) -o $(SOLUTION_EXE) $(MODULES_O) $(LFLAGS)
	$(MAKE) clean_o
	./$(SOLUTION_EXE)

%.o: %.c
	$(CC) $(FLAGS) -c $< -o $@

.PHONY: fmt
fmt:
	clang-format -i $(MODULES_C)

.PHONY: clean
clean: clean_o clean_exe clean_lib

.PHONY: clean_o
clean_o:
	rm -rf *.o

.PHONY: clean_exe
clean_exe:
	rm -rf $(SOLUTION_EXE)

.PHONY: clean_lib
clean_lib:
	$(MAKE) -C $(LIB_DIR) clean