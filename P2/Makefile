client:
	gcc -g -o cluster_client.c cluster_utilities.c inet_sockets.c -o client 

server:
	gcc -g -o  get_config.c inet_sockets.c parse_input.c

clean:
	rm -rf server
	rm -rf client
	rm -rf *.o
