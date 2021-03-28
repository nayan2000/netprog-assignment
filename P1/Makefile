CC=gcc
CFLAGS=-c -g
DEPS_PROG = *.o
all: shell
shell: $(DEPS_PROG)
	$(CC) $(DEPS_PROG) -o $@
	
%.o:%.c
	$(CC) $(CFLAGS) $^

clean:
	rm -rf shell
	rm -rf *.o