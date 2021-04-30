CC=gcc
CFLAGS=-c
DEPS_PROG = signal.o
all: driver
driver: $(DEPS_PROG)
	$(CC) $(DEPS_PROG) -o $@
	
%.o:%.c
	$(CC) $(CFLAGS) $^

clean:
	rm -rf driver
	rm -rf *.o