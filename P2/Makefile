binaries = client server
all: client server
client:
	gcc -g -Wall cluster_client.c client_utilities.c inet_sockets.c -o client 

server:
	gcc -g -Wall cluster_server.c parse_input.c inet_sockets.c get_config.c -o server

clean:
	rm -f $(binaries) *.o