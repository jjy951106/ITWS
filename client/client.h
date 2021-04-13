/* Single Thread */

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

#define SERVER "192.168.0.160" // test server
#define PORT 5005              // default port

#define ITERATION 10       // measure offset number
#define MEDIUM_TERM_SEC 0  //  s between receive and transmit
#define MEDIUM_TERM_NSEC 0 // ns between receive and transmit

/* requirement : 5ms */

#define BOUNDARY 5000000   // plus(ns)
#define BOUNDARY_ -5000000 // minus(ns)


/* print error */
static void err(const char *error);

/* select mode 1 2 3 */
int select_mode(int sock, int mode, struct sockaddr_in *server_addr, int protocol);

/* socket send */
void send_socket(int sock, struct sockaddr_in *server_addr, int protocol);

/* socket recv */
void recv_socket(int sock, struct msghdr *msg, struct sockaddr_in *server_addr, int protocol, struct timespec *T);

/* T3 sync initialized */
void initialized_T(int sock, struct sockaddr_in *server_addr, int protocol);

/* once offset */
void offset_calculated(int sock, int *offset, struct sockaddr_in *server_addr, int protocol);

/* iterative offset mean in adaptive deviations */
void iterative_offset_calculated(int sock, int32_t *offset, struct sockaddr_in *server_addr, int protocol);

/* once sync */
void mode_1(int sock, struct sockaddr_in *server_addr, int protocol);

/* sync loop */
void mode_2(int sock, struct sockaddr_in *server_addr, int protocol);

/* measure offset */
void mode_3(int sock, struct sockaddr_in *server_addr, int protocol);

/* TCP protocol */
int TCP_socket(struct sockaddr_in *server_addr, int mode, int protocol, int thr);

/* UDP protocol */
int UDP_socket(struct sockaddr_in *server_addr, int mode, int protocol, int thr);