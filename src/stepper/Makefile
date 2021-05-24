CC=gcc
CFLAGS=-Wall -pthread
INCLUDE=../libs
WIRINGPI=wiringPi
STEPPER=stepper
BLUETOOTH=bt
EXEC=main
BIN=../bin

defaults: make

make: $(EXEC).c
	$(CC) $(CFLAGS) -I $(INCLUDE) -c $(EXEC).c -o $(BIN)/$(EXEC).o
	$(CC) $(CFLAGS) -o $(BIN)/$(EXEC) $(BIN)/$(EXEC).o -L$(BIN)/shared -l$(STEPPER) -l$(BLUETOOTH) -l$(WIRINGPI) -lbluetooth