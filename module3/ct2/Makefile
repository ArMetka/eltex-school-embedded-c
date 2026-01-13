CC=gcc
FLAGS=-Wall -Wextra
LFLAGS=
DRIVER_MODULES_C=driver.c
DRIVER_MODULES_O=$(DRIVER_MODULES_C:.c=.o)
DRIVER_EXE=driver
TAXI_CLI_MODULES_C=taxi_cli.c
TAXI_CLI_MODULES_O=$(TAXI_CLI_MODULES_C:.c=.o)
TAXI_CLI_EXE=taxi_cli

all: clean
	$(MAKE) driver
	$(MAKE) taxi_cli
	./$(TAXI_CLI_EXE)

.PHONY: taxi_cli
taxi_cli: $(TAXI_CLI_MODULES_O)
	$(CC) -o $(TAXI_CLI_EXE) $(TAXI_CLI_MODULES_O) $(LFLAGS)
	$(MAKE) clean_o

.PHONY: driver
driver: $(DRIVER_MODULES_O)
	$(CC) -o $(DRIVER_EXE) $(DRIVER_MODULES_O) $(LFLAGS)
	$(MAKE) clean_o

%.o: %.c
	$(CC) $(FLAGS) -c $< -o $@

.PHONY: fmt
fmt:
	clang-format -i $(TAXI_CLI_MODULES_C) $(DRIVER_MODULES_C)

.PHONY: clean
clean: clean_o clean_exe

.PHONY: clean_o
clean_o:
	rm -rf *.o

.PHONY: clean_exe
clean_exe:
	rm -rf $(TAXI_CLI_EXE)
	rm -rf $(DRIVER_EXE)
