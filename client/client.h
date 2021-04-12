/* Single Thread */

#include <stdio.h>
#include <stdlib.h> // exit(0) 'normal' exit(1) 'error'
#include <string.h>
#include <stdint.h> // int32_t int64_t

#include <time.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <unistd.h>  // sleep usleep
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>

#include <errno.h>
#include "linux/errqueue.h"

#include <netdb.h> // domain address

#define SERVER "192.168.0.160" // test server
#define PORT 5005              // default port

#define ITERATION 10
#define MEDIUM_TERM_SEC 0
#define MEDIUM_TERM_NSEC 0 // ns between receive and transmit

/* requirement : 5ms */

#define BOUNDARY 5000000 // plus(ns)
#define BOUNDARY_ -5000000 // minus(ns)

static void err(const char *error);

int select_mode(int sock, int mode, struct sockaddr_in *server_addr, int protocol);

void send_socket(int sock, struct sockaddr_in *server_addr, int protocol);

void recv_socket(int sock, struct msghdr *msg, struct sockaddr_in *server_addr, int protocol, struct timespec *T);

void initialized_T(int sock, struct sockaddr_in *server_addr, int protocol);

void offset_calculated(int sock, int *offset, struct sockaddr_in *server_addr, int protocol);

void iterative_offset_calculated(int sock, int32_t *offset, struct sockaddr_in *server_addr, int protocol);

void mode_1(int sock, struct sockaddr_in *server_addr, int protocol);

void mode_2(int sock, struct sockaddr_in *server_addr, int protocol);

void mode_3(int sock, struct sockaddr_in *server_addr, int protocol);

int TCP_socket(struct sockaddr_in *server_addr, int mode, int protocol);

int UDP_socket(struct sockaddr_in *server_addr, int mode, int protocol);