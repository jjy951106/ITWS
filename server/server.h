/* Multi_Thread */

#include <stdio.h>
#include <stdlib.h> // exit(0) 'normal' exit(1) 'error'
#include <string.h>
#include <stdint.h> // int32_t int64_t
#include <assert.h>

#include <time.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <pthread.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <errno.h>
#include "linux/errqueue.h"

#define BACKLOG 100

#define MEDIUM_TERM_SEC 0
#define MEDIUM_TERM_NSEC 10000000 // nanosecond between receive and transmit

#define PORT 5005 // default port

#define FC_COMPS_PORT_UDP 5006

typedef struct udp_thread_factor{

    int sock;
    struct msghdr msg;
    struct sockaddr_in from_addr;

}udp_thread_factor;

int64_t _atoi(char *cdata);

static void err(const char *error);

int recv_socket(int sock, struct msghdr *msg, struct sockaddr_in *from_addr);

void *Server_Socket_Thread(void *arg);

void *UDP_Thread(void *args);

void *UDP_FC_COMPS_Thread(void *arg);

int TCP_server(struct sockaddr_in *server_addr);

int UDP_server(struct sockaddr_in *server_addr);