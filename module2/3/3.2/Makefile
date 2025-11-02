CC=gcc
FLAGS=-Wall -Wextra -std=c11
MODULES_C=ip4test.c
MODULES_H=ip4test.h
MODULES_O=$(MODULES_C:.c=.o)
SOLUTION_EXE=ip4test

all: clean
	make solution
	make run

solution: $(MODULES_O)
	$(CC) -o $(SOLUTION_EXE) $(MODULES_O) -lm
	make clean_o

%.o: %.c
	$(CC) $(FLAGS) -c $< -o $@

fmt:
	clang-format -i $(MODULES_C) $(MODULES_H)

run:
	./$(SOLUTION_EXE) 192.168.0.0/24 10000

clean: clean_o clean_exe

clean_o:
	rm -rf *.o

clean_exe:
	rm -rf $(SOLUTION_EXE)