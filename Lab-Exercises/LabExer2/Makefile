CC=gcc
CFLAGS=-c -g -Wall
DEPS_PROG = select_epoll_test.o
all: driver
driver: $(DEPS_PROG)
	$(CC) $(DEPS_PROG) -o $@
	
%.o:%.c
	$(CC) $(CFLAGS) $^

clean:
	rm -rf driver
	rm -rf *.o