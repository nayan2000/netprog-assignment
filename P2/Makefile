all: client server
client:
	gcc -g -Wall -o client cluster_client.c client_utilities.c inet_sockets.c

server:
	gcc -g -Wall -o server cluster_server.c parse_input.c inet_sockets.c get_config.c

clean:
	rm -rf server
	rm -rf *.o
	rm -rf client