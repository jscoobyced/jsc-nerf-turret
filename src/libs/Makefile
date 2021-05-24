CC=gcc
CFLAGS=-Wall -pthread
INCLUDE=../libs
WIRINGPI=wiringPi
BLUETOOTH=bt
STEPPER=stepper
BIN=../bin

defaults: make

make: ${STEPPER}.c ${BLUETOOTH}.c
	$(CC) $(CFLAGS) -c ${BLUETOOTH}.c -fPIC -pthread -o $(BIN)/shared/${BLUETOOTH}.o
	$(CC) $(CFLAGS) -shared -L. -L$(BIN)/shared -lbluetooth -Wl,-soname,lib${BLUETOOTH}.so,-rpath=. -o $(BIN)/shared/lib${BLUETOOTH}.so $(BIN)/shared/${BLUETOOTH}.o

	$(CC) $(CFLAGS) -c ${STEPPER}.c -fPIC -pthread -o $(BIN)/shared/${STEPPER}.o
	$(CC) $(CFLAGS) -shared -L. -L$(BIN)/shared -l$(WIRINGPI) -Wl,-soname,lib${STEPPER}.so,-rpath=. -o $(BIN)/shared/lib${STEPPER}.so $(BIN)/shared/${STEPPER}.o