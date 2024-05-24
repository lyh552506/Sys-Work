#include <assert.h>
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

void ToFile(char *src, char *dst) {
  char buff[512];
  char choice;
  bool IsCover = false, IsCat = false, Drop = false;
  if (access(src, F_OK) == -1) {
    printf("cp: cannot stat '%s': No such file or directory", src);
    exit(0);
  }
  if (access(dst, W_OK) != -1) {
    printf("current dst has same name: %s, Should We Cover? [Y/N]\n", dst);
    scanf("%c", &choice);
    getchar();
    if (choice == 'Y' || choice == 'y')
      IsCover = true;
    else if (choice == 'N' || choice == 'n') {
      printf("Should We Cat two files? [Y/N]\n");
      scanf("%c", &choice);
      getchar();
      if (choice == 'Y' || choice == 'y')
        IsCat = true;
      else if (choice == 'N' || choice == 'n') {
        printf("then we will drop this file\n");
        Drop = true;
      } else {
        perror("wrong input");
      }
    } else {
      perror("wrong input");
    }
  }
  if (!IsCover && !IsCat && !Drop) {
    int fd_dst = open(dst, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
    if (fd_dst == -1) {
      perror("fd");
      exit(0);
    }
    int fd_src = open(src, O_RDONLY);
    if (fd_src == -1) {
      perror("fd");
      exit(0);
    }
    ssize_t bufread = 0;
    while ((bufread = read(fd_src, buff, sizeof(buff))) > 0) {
      write(fd_dst, buff, bufread);
    }
    close(fd_dst);
    close(fd_src);
  } else if (IsCover && !IsCat && !Drop) {
    int fd_dst = open(dst, O_CREAT | O_WRONLY | O_TRUNC);
    int fd_src = open(src, O_RDONLY);
    ssize_t bufread = 0;
    while ((bufread = read(fd_src, buff, sizeof(buff))) > 0) {
      write(fd_dst, buff, bufread);
    }
    close(fd_dst);
    close(fd_src);
  } else if (!IsCover && IsCat && !Drop) {
    int fd_dst = open(dst, O_WRONLY | O_APPEND);
    int fd_src = open(src, O_RDONLY);
    ssize_t bufread = 0;
    while ((bufread = read(fd_src, buff, sizeof(buff))) > 0) {
      write(fd_dst, buff, bufread);
    }
    close(fd_dst);
    close(fd_src);
  } else if (!IsCover && !IsCat && Drop) {
    return;
  } else {
    assert(0);
  }
}

void DirToDir(char *src, char *dst) {
  DIR *dir = NULL, *subdir = NULL;
  struct dirent *read = NULL;
  struct stat *st = NULL;
  if ((dir = opendir(src)) == NULL) {
    printf("cp: cannot stat '%s': No such file or directory", src);
    exit(0);
  }
  if (access(dst, W_OK) == -1) {
    mkdir(dst, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  }
  if (stat(dst, st) != -1) {
    if (S_ISREG(st->st_mode)) {
      perror("dest is a file!");
      exit(0);
    }
  }
  while ((read = readdir(dir)) != NULL) {
    char *name = read->d_name;
    if (name[0] == '.')
      continue;
    char file_src_path[1024], file_dst_path[1024];
    sprintf(file_src_path, "%s%s%s", src, "/", name);
    sprintf(file_dst_path, "%s%s%s", dst, "/", name);
    if ((subdir = opendir(file_src_path)) != NULL) {
      DirToDir(file_src_path, file_dst_path);
      continue;
    }
    ToFile(file_src_path, file_dst_path);
  }
}

int main(int argc, char *argv[]) {
  char option;
  char src[MAX] = {0};
  char dst[MAX] = {0};
  bool recursive = false, IsDir = false; //是否递归复制
  if (argc < 2) {
    for (int i = 0; i < argc; i++)
      printf("%s\n", argv[i]);
    perror("errer: at least tow argues");
    return 0;
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
  if (opendir(src) != NULL && recursive == true) {
    DirToDir(src, dst);
  } else if (opendir(src) == NULL && opendir(dst) != NULL) {
    const char *file_name = strrchr(src, '/');
    if (file_name) {
      sprintf(dst, "%s%s", dst, file_name);
      ToFile(src, dst);
    }
  } else {
    ToFile(src, dst);
  }
}