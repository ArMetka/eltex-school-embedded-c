CC=gcc
FLAGS=-Wall -Wextra
LFLAGS=
CLIENT_MODULES_C=client.c
CLIENT_MODULES_O=$(CLIENT_MODULES_C:.c=.o)
CLIENT_EXE=client
SERVER_MODULES_C=server.c
SERVER_MODULES_O=$(SERVER_MODULES_C:.c=.o)
SERVER_EXE=server

all: clean
	$(MAKE) client
	$(MAKE) server
	./$(SERVER_EXE) 55555

.PHONY: server
server: $(SERVER_MODULES_O)
	$(CC) -o $(SERVER_EXE) $(SERVER_MODULES_O) $(LFLAGS)
	$(MAKE) clean_o

.PHONY: client
client: $(CLIENT_MODULES_O)
	$(CC) -o $(CLIENT_EXE) $(CLIENT_MODULES_O) $(LFLAGS)
	$(MAKE) clean_o

%.o: %.c
	$(CC) $(FLAGS) -c $< -o $@

.PHONY: fmt
fmt:
	clang-format -i $(SERVER_MODULES_C) $(CLIENT_MODULES_C)

.PHONY: clean
clean: clean_o clean_exe

.PHONY: clean_o
clean_o:
	rm -rf *.o

.PHONY: clean_exe
clean_exe:
	rm -rf $(SERVER_EXE)
	rm -rf $(CLIENT_EXE)
