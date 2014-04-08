all:
	gcc client.c -o client
	gcc server.c -pthread -o server
