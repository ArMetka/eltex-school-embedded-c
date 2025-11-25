CC=gcc
FLAGS=-Wall -Wextra
LFLAGS=-L. -lipc
CLIENT_MODULES_C=client.c
CLIENT_MODULES_O=$(CLIENT_MODULES_C:.c=.o)
CLIENT_EXE=client
SERVER_MODULES_C=server.c
SERVER_MODULES_O=$(SERVER_MODULES_C:.c=.o)
SERVER_EXE=server
LIB_MODULES_C=ipc.c
LIB_MODULES_O=$(LIB_MODULES_C:.c=.o)
LIB_NAME=libipc.so

all: clean
	$(MAKE) lib
	$(MAKE) server
	LD_LIBRARY_PATH=$(PWD) ./$(SERVER_EXE)

.PHONY: lib
lib:
	$(CC) $(FLAGS) -fPIC -c $(LIB_MODULES_C)
	$(CC) -shared $(LIB_MODULES_O) -o $(LIB_NAME)

.PHONY: solution
server: $(SERVER_MODULES_O) lib
	$(CC) -o $(SERVER_EXE) $(SERVER_MODULES_O) $(LFLAGS)
	$(MAKE) clean_o

.PHONY: solution
client: $(CLIENT_MODULES_O) lib
	$(CC) -o $(CLIENT_EXE) $(CLIENT_MODULES_O) $(LFLAGS)
	$(MAKE) clean_o

%.o: %.c
	$(CC) $(FLAGS) -c $< -o $@

.PHONY: fmt
fmt:
	clang-format -i $(SERVER_MODULES_C) $(CLIENT_MODULES_C) $(LIB_MODULES_C)

.PHONY: clean
clean: clean_o clean_exe clean_lib

.PHONY: clean_o
clean_o:
	rm -rf *.o

.PHONY: clean_exe
clean_exe:
	rm -rf $(SERVER_EXE)
	rm -rf $(CLIENT_EXE)

.PHONY: clean_lib
clean_lib:
	rm -rf $(LIB_NAME)
