binaries = client server
all: client server
client:
	gcc -g -Wall msgq_client.c -o client 

server:
	gcc -g -Wall msgq_server.c hashmap.c -o server

clean:
	rm -f $(binaries) *.o