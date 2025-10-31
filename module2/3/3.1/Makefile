CC=gcc
FLAGS=-Wall -Wextra -std=c11
MODULES_C=chstat.c
MODULES_H=chstat.h
MODULES_O=$(MODULES_C:.c=.o)
SOLUTION_EXE=chstat

all: clean
	make solution
	./$(SOLUTION_EXE)

solution: $(MODULES_O)
	$(CC) -o $(SOLUTION_EXE) $(MODULES_O)
	make clean_o

%.o: %.c
	$(CC) $(FLAGS) -c $< -o $@

fmt:
	clang-format -i $(MODULES_C) $(MODULES_H)

clean: clean_o clean_exe

clean_o:
	rm -rf *.o

clean_exe:
	rm -rf $(SOLUTION_EXE)