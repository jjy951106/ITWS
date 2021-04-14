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

/* term between receive and transmit */
#define MEDIUM_TERM_SEC 0
#define MEDIUM_TERM_NSEC 10000000 /* 10,000,000 */

/* default mc port 5005
   default fc port 5006 */
#define PORT 5005 
#define FC_COMPS_PORT_UDP 5006

typedef struct udp_thread_factor{

    int sock;
    struct msghdr msg;
    struct sockaddr_in from_addr;

    /* Offset between FC and MC */
    double Compenstate_FC_MC;

}udp_thread_factor;

typedef struct fc_offset{
    int64_t max;
    int64_t min;
    int64_t fc_comps_buf[30];
    int count;
    int count_bound;
    int sync_during_ignored; /* Ignore the first 5 */
}fc_offset;

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

/* UDP sync Function */
void UDP_Function(void *args);

void UDP_FC_COMPS_Fuction(void *args, fc_offset *fc, char *buf, double *Compenstate_FC_MC);

/* TCP protocol */
int TCP_server(struct sockaddr_in *server_addr);

/* UDP protocol */
int UDP_server(struct sockaddr_in *server_addr);