#include <dirent.h>
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
#define SIZE 1024

typedef struct File {
  char name[100];      //文件名
  char user[100];      //文件所有者
  char belong[100];    //文件所有者所在组
  char last_time[100]; //文件最后修改时间
  off_t size;          //文件大小
  nlink_t links;       //文件硬链接数或目录子目录数
  char type[10];       //文件类型
  bool IsSymLink = true;
  void show() {
    printf("%s\t%ld\t%s\t%s\t%ld\t%s\t%s\n", type, links, user, belong, size,
           last_time, name);
  }
} file;

void decode_type(file *f, mode_t mode) {
  char name[10];
  for (int i = 0; i < sizeof(name); i++)
    name[i] = '-';
  if (S_ISDIR(mode))
    name[0] = 'd';
  if (mode & S_IRUSR)
    name[1] = 'r';
  if (mode & S_IWUSR)
    name[2] = 'w';
  if (mode & S_IXUSR)
    name[3] = 'x';
  if (mode & S_IRGRP)
    name[4] = 'r';
  if (mode & S_IWGRP)
    name[5] = 'w';
  if (mode & S_IXGRP)
    name[6] = 'x';
  if (mode & S_IROTH)
    name[7] = 'r';
  if (mode & S_IWOTH)
    name[8] = 'w';
  if (mode & S_IXOTH)
    name[9] = 'x';
  memcpy(f->type, name, sizeof(name));
}

void decode_links(file *f, nlink_t links) { f->links = links; }

void decode_userName(file *f, uid_t uid, gid_t gid) {
  struct passwd *ps = getpwuid(uid);
  struct group *gp = getgrgid(gid);
  memcpy(f->user, ps->pw_name, strlen(ps->pw_name));
  memcpy(f->belong, gp->gr_name, strlen(gp->gr_name));
}

void decode_fileSize(file *f, off_t size) { f->size = size; }

void decode_time(file *f, time_t time) {
  struct tm lt;
  localtime_r(&time, &lt);
  char ti[100];
  strftime(ti, sizeof(ti), "%b %d %H:%M", &lt);
  // printf("%s\n",ti);
  memcpy(f->last_time, ti, sizeof(ti));
}

void decode_name(file *f, char *name, char *pwd) {
  memcpy(f->name, name, strlen(name));
  if (f->IsSymLink) {
    char tmp[100] = {0};
    ssize_t len = readlink(pwd, tmp, sizeof(tmp) - 1);
    if (len != -1) {
      strcat(f->name, " -> ");
      strcat(f->name, tmp);
    }
    f->name[strlen(f->name)] = '\0';
    return;
  }
  f->name[strlen(name)] = '\0';
}

int main(int argc, char *argv[]) {
  char cwd[SIZE];
  char option;
  bool flag_a = false, flag_l = false, flag_L = false;
  DIR *dir = NULL;
  struct dirent *read = NULL;
  if (argc == 1 && getcwd(cwd, sizeof(cwd)) == NULL) {
    perror("getwpd");
    return -1;
  } else if (argc > 1) {
    memcpy(cwd, argv[1], strlen(argv[1])+1);
  }
  if ((dir = opendir(cwd)) == NULL) {
    perror("opendir");
    return -1;
  }
  while ((option = getopt(argc, argv, "alL")) != -1) {
    switch (option) {
    case 'a':
      flag_a = true;
      break;
    case 'l':
      flag_l = true;
      break;
    case 'L':
      flag_L = true;
      break;
    case '?':
      perror("unknown option!");
      // return -1;
    }
  }
  // equal to: ls
  if (!flag_a && !flag_l) {
    while ((read = readdir(dir)) != NULL) {
      if (read->d_name[0] != '.')
        printf("%s\n", read->d_name);
    }
    return 1;
  }
  // equal to: ls -l
  if (!flag_a && flag_l) {
    int total = 0;
    file *f = (file *)malloc(sizeof(file));
    while ((read = readdir(dir)) != NULL) {
      bool isLink = false;
      char pwd[SIZE];
      f->IsSymLink = true;
      if (read->d_name[0] == '.')
        continue;
      struct stat *buf = (struct stat *)malloc(sizeof(struct stat));
      //获取文件的绝对地址
      memcpy(pwd, cwd, sizeof(cwd));
      strcat(pwd, "/");
      strcat(pwd, read->d_name);
      //获取stat状态
      lstat(pwd, buf);
      if (!flag_L || !S_ISLNK(buf->st_mode)) {
        f->IsSymLink = false;
        stat(pwd, buf);
      }
      //获取type
      decode_type(f, buf->st_mode);
      decode_links(f, buf->st_nlink);
      decode_fileSize(f, buf->st_size);
      decode_userName(f, buf->st_uid, buf->st_gid);
      decode_time(f, buf->st_mtime);
      decode_name(f, read->d_name, pwd);
      total += buf->st_size;
      free(buf);
      f->show();
    }
    printf("%dk", (total) / 1024);
  }

  // equal to:ls -al
  if (flag_a && flag_l) {
    int total = 0;
    file *f = (file *)malloc(sizeof(file));
    while ((read = readdir(dir)) != NULL) {
      bool isLink = false;
      char pwd[SIZE];
      f->IsSymLink = true;
      struct stat *buf = (struct stat *)malloc(sizeof(struct stat));
      //获取文件的绝对地址
      memcpy(pwd, cwd, sizeof(cwd));
      strcat(pwd, "/");
      strcat(pwd, read->d_name);
      //获取stat状态
      lstat(pwd, buf);
      if (!flag_L || !S_ISLNK(buf->st_mode)) {
        stat(pwd, buf);
        f->IsSymLink = false;
      }
      //获取type
      decode_type(f, buf->st_mode);
      decode_links(f, buf->st_nlink);
      decode_fileSize(f, buf->st_size);
      decode_userName(f, buf->st_uid, buf->st_gid);
      decode_time(f, buf->st_mtime);
      decode_name(f, read->d_name, pwd);
      total += buf->st_size;
      free(buf);
      f->show();
    }
    printf("%dk", (total) / 1024);
  }

  closedir(dir);
}