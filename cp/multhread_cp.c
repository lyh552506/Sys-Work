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
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#define DST "/home/lyh"
#define SIZE 1024
#define MYCP "/home/lyh_irie/cpp_work/cp/cp"
#define MYLS "/home/lyh_irie/cpp_work/ls/myls"
#define MYMAIN "/home/lyh_irie/cpp_work/cp/multhread_cp"

void multi_cp(char *cwd) {
  DIR *dir = NULL;
  char file_name[SIZE];
  struct dirent *read = NULL;
  bool IsDir = false;
  pid_t pid_p, pid_c;
  struct stat st;
  if (access(cwd, F_OK) != -1) {
    if ((dir = opendir(cwd)) != NULL) {
      while ((read = readdir(dir)) != NULL) {
        if (read->d_name[0] == '.')
          continue;
        sprintf(file_name, "%s%s%s", cwd, "/", read->d_name);
        if (lstat(file_name, &st) != -1) {
          if (S_ISREG(st.st_mode)) {
            pid_p = fork();
            if (pid_p < 0) {
              perror("No Such File");
              exit(0);
            } else if (pid_p == 0) {
              execl(MYCP, file_name, (char *)DST, (char *)NULL);
            } else {
              waitpid(pid_p, NULL, 0);
            }
          } else if (S_ISDIR(st.st_mode)) {
            pid_p = fork();
            if (pid_p < 0) {
              perror("No Such File");
              exit(0);
            } else if (pid_p == 0) {
              execl(MYMAIN, file_name, (char *)DST, (char *)NULL);
            } else {
              waitpid(pid_p, NULL, 0);
            }
          }
        }
      }
    } else {
      pid_p = fork();
      if (pid_p < 0) {
        perror("No Such File");
        exit(0);
      } else if (pid_p == 0) {
        execl(MYCP, cwd, (char *)DST, (char *)NULL);
      } else {
        waitpid(pid_p, NULL, 0);
      }
    }
  } else {
    perror("No Such File");
  }
}

int main(int argc, char *argv[]) {
  char cwd[SIZE];
  char option;
  pid_t pid_p;
  if (argc == 1 && getcwd(cwd, sizeof(cwd)) == NULL) {
    perror("getwpd");
    return -1;
  } else if (argc ==3) {
    memcpy(cwd, argv[1], strlen(argv[1]) + 1);
  } else if(argc ==2){
    memcpy(cwd, argv[0], strlen(argv[0]) + 1);
  }
  multi_cp(cwd);
}