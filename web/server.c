#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define SIZE 1024

int main() {
  int sockfd = 0, accpet_fd = 0,tmp;
  struct sockaddr_in serv_addr;
  char rev[SIZE];
  memset(rev, 0, SIZE);
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Can Not Create socket");
    exit(0);
  }
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(8080);
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  if ((bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0) {
    perror("Bind Failed");
    exit(0);
  }
  if ((listen(sockfd, 10)) < 0) {
    perror("Listen Failed");
    exit(0);
  }
  if ((accpet_fd = accept(sockfd, (struct sockaddr *)NULL, NULL)) < 0) {
    perror("Accept Failed");
    exit(0);
  }
  while ((tmp=recv(accpet_fd, rev, SIZE, 0))>0) {
    printf("Server Receive: %s\n", rev);
    memset(rev, 0, SIZE);
  }
  close(sockfd);
  close(accpet_fd);
}