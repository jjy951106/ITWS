/* Multi_Thread */

#include <stdio.h>
#include <stdlib.h> // exit(0) 'normal' exit(1) 'error'
#include <string.h>
#include <assert.h>

#include <pthread.h>
#include <unistd.h>

#include <errno.h>
#include "linux/errqueue.h"

#include <netdb.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>

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

/* character -> number (custom) no limit size */
int64_t _atoi(char *cdata);

/* print error */
static void err(const char *error);

/* socket recv */
int recv_socket(int sock, struct msghdr *msg, struct sockaddr_in *from_addr);

/* TCP sync Thread */
void *Server_Socket_Thread(void *arg);

/* UDP sync Thread */
void *UDP_Thread(void *args);

/* UDP FC sync Thread */
void *UDP_FC_COMPS_Thread(void *arg);

/* TCP protocol */
int TCP_server(struct sockaddr_in *server_addr);

/* UDP protocol */
int UDP_server(struct sockaddr_in *server_addr);