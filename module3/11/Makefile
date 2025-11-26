CC=gcc
FLAGS=-Wall -Wextra
MODULES_C=solution.c
MODULES_O=$(MODULES_C:.c=.o)
SOLUTION_EXE=solution

all: clean
	$(MAKE) solution
	$(MAKE) run

.PHONY: solution
solution: $(MODULES_O)
	$(CC) -o $(SOLUTION_EXE) $(MODULES_O)
	$(MAKE) clean_o

.PHONY: run
run:
	./$(SOLUTION_EXE)

%.o: %.c
	$(CC) $(FLAGS) -c $< -o $@

.PHONY: fmt
fmt:
	clang-format -i $(MODULES_C)

.PHONY: clean
clean: clean_o clean_exe

.PHONY: clean_o
clean_o:
	rm -rf *.o

.PHONY: clean_exe
clean_exe:
	rm -rf $(SOLUTION_EXE)
