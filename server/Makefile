all: server

server: main_server.c server.c
	gcc -pthread -o server main_server.c server.c

server.out: main_server.o server.o
	gcc -pthread -o server main_server.o server.o

main_server.o: main_server.c
	gcc -c main_server.c

server.o: server.h server.c
	gcc -c server.c

clean:
	rm -f server main_server.o server.o