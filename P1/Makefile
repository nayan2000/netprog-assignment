CC=gcc
CFLAGS=-c
DEPS_PROG = shell.o job_control.o hashmap.o job_table.o exec_command.o parse_command.o
all: shell
shell: $(DEPS_PROG)
	$(CC) $(DEPS_PROG) -o $@
	
%.o:%.c
	$(CC) $(CFLAGS) $^

clean:
	rm -rf shell
	rm -rf *.o