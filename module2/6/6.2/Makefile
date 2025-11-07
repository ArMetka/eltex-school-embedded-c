CC=gcc
FLAGS=-Wall -Wextra -std=c11
LFLAGS=-L. -lcontact_db
LIB_MODULES_C=contact_db.c
MODULES_C=contact_db_cli.c
MODULES_H=contact_db.h
LIB_MODULES_O=$(LIB_MODULES_C:.c=.o)
MODULES_O=$(MODULES_C:.c=.o)
LIB_NAME=libcontact_db.so
SOLUTION_EXE=contact_db_cli

all: clean
	make lib
	make solution

lib:
	$(CC) $(FLAGS) -fPIC -c $(LIB_MODULES_C)
	$(CC) -shared $(LIB_MODULES_O) -o $(LIB_NAME)
	make clean_o

solution: $(MODULES_O)
	$(CC) -o $(SOLUTION_EXE) $(MODULES_O) $(LFLAGS)
	make clean_o
	LD_LIBRARY_PATH=$(PWD) ./$(SOLUTION_EXE)

%.o: %.c
	$(CC) $(FLAGS) -c $< -o $@

fmt:
	clang-format -i $(MODULES_C) $(MODULES_H)

clean: clean_o clean_exe clean_lib

clean_o:
	rm -rf *.o

clean_exe:
	rm -rf $(SOLUTION_EXE)

clean_lib:
	rm -rf $(LIB_NAME)