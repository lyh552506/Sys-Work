#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main() {
  int sockfd = 0;
  struct sockaddr_in serv_addr;
  const char *send_mess = "I am your client";
  memset(&serv_addr, 0, sizeof(struct sockaddr_in));
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Can Not Creat");
    exit(0);
  }
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port =htons(8888);
  serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  if ((connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0) {
    perror("Can Not Connect");
    exit(0);
  }
  printf("Now We Successfully connect to the server");
  if ((send(sockfd, send_mess, strlen(send_mess)+1, 0)) < 0) {
    perror("Send Accur Failure");
    exit(0);
  }
  close(sockfd);
}