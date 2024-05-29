#include <arpa/inet.h>
#include <cstddef>
#include <cstdlib>
// #include <event.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#define SIZE 1024
//TODO 
//should deal different situation with http
void Handle_Http(int fd) {
  int buf;
  char rev[SIZE], method[SIZE], url[SIZE], version[SIZE], headers[SIZE];
  struct stat stat_buf;
  memset(rev, 0, SIZE);
  buf = recv(fd, rev, SIZE, 0);
  if (buf < 0) {
    perror("WRONG WITH REV");
    exit(0);
  }
  sscanf(rev, "%s%s%s", method, url, version);
  // Print the parsed information
  printf("Method: %s\n", method);
  printf("URL: %s\n", url);
  printf("HTTP Version: %s\n\n", version);
  if (strcmp(url, "/") == 0) {
    int file = open("/home/lyh_irie/cpp_work/web/index.html", O_RDONLY);
    if (file < 0) {
      perror("NO HTML FILE");
      exit(0);
    }
    if (stat("/home/lyh_irie/cpp_work/web/index.html", &stat_buf) < 0) {
      perror("stat");
      exit(1);
    }
    sprintf(headers,
            "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: "
            "%ld\r\n\r\n",
            stat_buf.st_size);
    //发送状态行和消息报头
    send(fd, headers, strlen(headers), 0);

    sendfile(fd, file, NULL, stat_buf.st_size);
  } else if (strcmp(url, "/cat.png") == 0) {
    int file = open("/home/lyh_irie/cpp_work/web/cat.png", O_RDONLY);
    if (file < 0) {
      perror("NO HTML FILE");
      exit(0);
    }
    if (stat("/home/lyh_irie/cpp_work/web/cat.png", &stat_buf) < 0) {
      perror("stat");
      exit(1);
    }
    sprintf(headers,
            "HTTP/1.1 200 OK\r\nContent-Type: image/png\r\nContent-Length: "
            "%ld\r\n\r\n",
            stat_buf.st_size);
    //发送状态行和消息报头
    send(fd, headers, strlen(headers), 0);

    sendfile(fd, file, NULL, stat_buf.st_size);
  } else {
    char response[] =
        "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nHello, World!";
    send(fd, response, strlen(response), 0);
  }
}

int main() {
  int sockfd = 0, accpet_fd = 0;
  struct sockaddr_in serv_addr;
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
  while (1) {
    if ((accpet_fd = accept(sockfd, (struct sockaddr *)NULL, NULL)) < 0) {
      perror("Accept Failed");
      exit(0);
    }
    // pid_t pid = fork();
    // if (pid < 0) {
    //   perror("fork failed");
    //   exit(0);
    // } else if (pid == 0) {
    Handle_Http(accpet_fd);
    close(accpet_fd);
    //    exit(0);
    // } else {
    //   close(accpet_fd);
  }

  close(sockfd);
}