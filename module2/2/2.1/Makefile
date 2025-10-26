CC=gcc
FLAGS=-Wall -Wextra -std=c99
MODULES_C=contact_db.c main.c
MODULES_H=contact_db.h
MODULES_O=$(MODULES_C:.c=.o)
SOLUTION_EXE=solution

all: clean
	make solution

solution: $(MODULES_O)
	$(CC) -o $(SOLUTION_EXE) $(MODULES_O)
	make clean_o
	./$(SOLUTION_EXE)

%.o: %.c
	$(CC) $(FLAGS) -c $< -o $@

fmt:
	clang-format -i $(MODULES_C) $(MODULES_H)

clean: clean_o clean_exe

clean_o:
	rm -rf *.o

clean_exe:
	rm -rf $(SOLUTION_EXE)