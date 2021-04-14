all: server client

server: ./server/main_server.c ./server/server.c
	gcc -pthread -o server main_server.c server.c

client: ./client/main_client.c ./client/client.c
	gcc -o client ./client/main_client.c ./client/client.c

clean:
	rm -f server client