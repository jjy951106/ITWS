#include "server.h"

int main(int argc, char *argv[]){
  
  struct sockaddr_in server_addr;
  
  int protocol = 0;/* filename port */

  char* intial = "initialization";

  memset(&server_addr, '\0', sizeof(server_addr));

  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_port = htons(PORT);

  if(argc >= 2) server_addr.sin_port = htons(atoi(argv[1]));

  if(argc == 3 && atoi(argv[2]) == 1) protocol = 1;

  if (argc > 3) {
    printf("Input exceeded\n");
    return 0;
  }
  
  if (protocol == 0)
    UDP_server(&server_addr);
  else
    TCP_server(&server_addr);

  return 0;
}