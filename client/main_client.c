#include "client.h"

int main(int argc, char *argv[]){

    int mode = 1 /* default mode_1 */, protocol = 0; /* default protocol UDP */

    int32_t thr = 5000000; // threshold default 5,000,000 ns

    struct sockaddr_in server_addr;

    char *IPbuffer;
    struct hostent *host_entry;

    memset(&server_addr, '\0', sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER /* 192.168.0.160 */);
    server_addr.sin_port = htons(PORT /* 5005 */);

    system("sudo timedatectl set-ntp false");

    if(argc >= 2) mode = atoi(argv[1]);   

    if(argc >= 3){
        // To retrieve host information
        host_entry = gethostbyname(argv[2]);

        // To convert an Internet network
        // address into ASCII string
        IPbuffer = inet_ntoa(*((struct in_addr*)host_entry->h_addr_list[0]));

        server_addr.sin_addr.s_addr = inet_addr(IPbuffer);
    }

    if(argc >= 4) server_addr.sin_port = htons(atoi(argv[3]));

    /* (defalt, 1) : (UDP, TCP) */
    if(argc >= 5 && atoi(argv[4]) == 1) protocol = 1;

    /* threshold default 5ms*/
    if(argc < 6) thr = BOUNDARY;

    /* threshold input ms */
    if(argc == 6) thr = atoi(argv[5]) * 1000000; 

    if (argc > 6) {
		printf("Input exceeded\n");
		return 0;
	}

    if (protocol == 0)
		UDP_socket(&server_addr, mode, protocol, thr);

	else
		TCP_socket(&server_addr, mode, protocol, thr);

    return 0;

}
