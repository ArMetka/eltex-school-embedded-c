CC=gcc
FLAGS=-Wall -Wextra
LFLAGS=-L. -lipc
CONSUMER_MODULES_C=consumer.c
CONSUMER_MODULES_O=$(CONSUMER_MODULES_C:.c=.o)
CONSUMER_EXE=consumer
PRODUCER_MODULES_C=producer.c
PRODUCER_MODULES_O=$(PRODUCER_MODULES_C:.c=.o)
PRODUCER_EXE=producer
LIB_MODULES_C=ipc.c
LIB_MODULES_O=$(LIB_MODULES_C:.c=.o)
LIB_NAME=libipc.a

all: clean
	$(MAKE) lib
	$(MAKE) producer
	$(MAKE) consumer

.PHONY: lib
lib: $(LIB_MODULES_O)
	ar rcs $(LIB_NAME) $(LIB_MODULES_O)

.PHONY: producer
producer: $(PRODUCER_MODULES_O) lib
	$(CC) -o $(PRODUCER_EXE) $(PRODUCER_MODULES_O) $(LFLAGS)
	$(MAKE) clean_o

.PHONY: consumer
consumer: $(CONSUMER_MODULES_O) lib
	$(CC) -o $(CONSUMER_EXE) $(CONSUMER_MODULES_O) $(LFLAGS)
	$(MAKE) clean_o

%.o: %.c
	$(CC) $(FLAGS) -c $< -o $@

.PHONY: fmt
fmt:
	clang-format -i $(PRODUCER_MODULES_C) $(CONSUMER_MODULES_C) $(LIB_MODULES_C)

.PHONY: clean
clean: clean_o clean_exe clean_lib

.PHONY: clean_o
clean_o:
	rm -rf *.o

.PHONY: clean_exe
clean_exe:
	rm -rf $(PRODUCER_EXE)
	rm -rf $(CONSUMER_EXE)

.PHONY: clean_lib
clean_lib:
	rm -rf $(LIB_NAME)
