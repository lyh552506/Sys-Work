#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#define DEV_NAME "/dev/lyh_dev_2"

int main() {
  int fd;
  char *name;
  char *out;
  name = (char *)malloc(1024);
  out = (char *)malloc(1024);
  strcpy(name, "I am LYH");
  fd = open(DEV_NAME, O_RDWR);
  if (fd < 0) {
    printf("%d\n", fd);
    perror("Can Not Open File");
    return 1;
  }
  int num = write(fd, name, strlen(name));
  if (num < 0) {
    perror("File is Not Writable\n");
    return 0;
  }
  printf("Success Open File!\n");
  num = read(fd, out, 1024);
  if (num < 0) {
    perror("File is Not Readable\n");
    return 0;
  }
  printf("Success Read the File! Output is: %s\n", out);
  close(fd);
}