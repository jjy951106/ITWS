all: server

server:
	cd server;\
	gcc -pthread -o server server.c main_server.c
