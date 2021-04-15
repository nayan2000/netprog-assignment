CC=gcc
CFLAGS=-c -g
DEPS_PROG = *.o
all: prefork
prefork: $(DEPS_PROG)
	$(CC) $(DEPS_PROG) -o $@
	
%.o:%.c
	$(CC) $(CFLAGS) $^

clean:
	rm -rf prefork
	rm -rf *.o