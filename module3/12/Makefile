CC=gcc
FLAGS=-Wall -Wextra
CLIENT_MODULES_C=client.c
CLIENT_MODULES_O=$(CLIENT_MODULES_C:.c=.o)
CLIENT_EXE=client

all: clean
	$(MAKE) client

.PHONY: client
client: $(CLIENT_MODULES_O)
	$(CC) -o $(CLIENT_EXE) $(CLIENT_MODULES_O)
	$(MAKE) clean_o

%.o: %.c
	$(CC) $(FLAGS) -c $< -o $@

.PHONY: fmt
fmt:
	clang-format -i $(CLIENT_MODULES_C)

.PHONY: clean
clean: clean_o clean_exe

.PHONY: clean_o
clean_o:
	rm -rf *.o

.PHONY: clean_exe
clean_exe:
	rm -rf $(CLIENT_EXE)
