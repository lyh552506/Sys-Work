#include <dirent.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#define MAX 100

void FileToFile(char *src, char *dst) {
  char buff[512];
  if (access(src, F_OK) == -1) {
    printf("cp: cannot stat '%s': No such file or directory", src);
    exit(0);
  }
  int fd_dst = open(dst, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
  int fd_src = open(src, O_RDONLY);
  ssize_t bufread=0;
  while((bufread=read(fd_src,buff,sizeof(buff)))>0){
    write(fd_dst, buff, bufread);
  }
  close(fd_dst);
  close(fd_src);
}

void DirToDir(char *src, char *dst) {
  DIR* dir=NULL;
  struct dirent* read;
  if((dir=opendir(src))==NULL){
    printf("cp: cannot stat '%s': No such file or directory", src);
    exit(0);
  }
  while((read = readdir(dir)) != NULL){
    char *name=read->d_name;
    char file_path[1024];
    snprintf(file_path, sizeof(file_path), src, name);

  }
}

int main(int argc, char *argv[]) {
  char option;
  char src[MAX] = {0};
  char dst[MAX] = {0};
  bool recursive = false, IsDir = false; //是否递归复制
  if (argc < 3) {
    perror("errer: at least tow argues");
  }
  while ((option = getopt(argc, argv, "r")) != -1) {
    switch (option) {
    case 'r':
      recursive = true;
      break;
    case '?':
      perror("unknown option!");
      return -1;
    }
  }
  memcpy(src, argv[argc - 2], strlen(argv[argc - 2]));
  memcpy(dst, argv[argc - 1], strlen(argv[argc - 1]));
  if (opendir(src) != NULL) {
    DirToDir(src, dst);
  } else {
    FileToFile(src, dst);
  }
}