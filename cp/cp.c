#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#define MAX 100

void FileToFile(char *src, char *dst) {
  if (access(src, F_OK) == -1) {
    printf("cp: cannot stat '%s': No such file or directory", src);
    exit(0);
  }
  int fd_dst = open(dst, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
}

void DirToDir(char *src, char *dst) {}

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