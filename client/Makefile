all: client

client: main_client.c client.c
	gcc -o client main_client.c client.c

client.out: main_client.o client.o
	gcc -o client main_client.o client.o

main_client.o: main_client.c
	gcc -c main_client.c

client.o: client.h client.c
	gcc -c client.c

clean:
	rm -f client main_client.o client.o