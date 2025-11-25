CC=gcc
FLAGS=-Wall -Wextra
LFLAGS=-L. -lipc
CLIENT_MODULES_C=client.c
CLIENT_MODULES_O=$(CLIENT_MODULES_C:.c=.o)
CLIENT_EXE=client
LIB_MODULES_C=ipc.c
LIB_MODULES_O=$(LIB_MODULES_C:.c=.o)
LIB_NAME=libipc.a

all: clean
	$(MAKE) lib
	$(MAKE) client
	./$(CLIENT_EXE)

.PHONY: lib
lib: $(LIB_MODULES_O)
	ar rcs $(LIB_NAME) $(LIB_MODULES_O)

.PHONY: client
client: $(CLIENT_MODULES_O) lib
	$(CC) -o $(CLIENT_EXE) $(CLIENT_MODULES_O) $(LFLAGS)
	$(MAKE) clean_o

%.o: %.c
	$(CC) $(FLAGS) -c $< -o $@

.PHONY: fmt
fmt:
	clang-format -i $(CLIENT_MODULES_C) $(LIB_MODULES_C)

.PHONY: clean
clean: clean_o clean_exe clean_lib

.PHONY: clean_o
clean_o:
	rm -rf *.o

.PHONY: clean_exe
clean_exe:
	rm -rf $(CLIENT_EXE)

.PHONY: clean_lib
clean_lib:
	rm -rf $(LIB_NAME)
