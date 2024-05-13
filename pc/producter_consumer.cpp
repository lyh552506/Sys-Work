#include <cstddef>
#include <cstdlib>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include"sys/sem.h"
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_NUM 5
#define BUFFER_SIZE 27
#define PRODUCER_NUM 1
#define CONSUMER_NUM 1
#define FILE_PATH "/mnt/e/cpp_work/producerN.txt"
char buff[BUFFER_SIZE];
pthread_mutex_t mutex_pro;
pthread_mutex_t mutex_con;

struct BufferPool {
  char buffer[BUFFER_SIZE];
  int stat =
      0; // stat=0表示缓冲区未被生产者使用，可分配但不可消费；stat=1表示缓冲区已被生产者使用，不可分配但可消费
};

void delay() {
  srand((unsigned)time(NULL));
  int t = rand() % 9000 + 4000;
  for (int i = 0; i < t; i++) {
    for (int j = 0; j < t; j++) {
      i = i + 1;
      i = i - 1;
    }
  }
}

void Producer(void *arg) {
  //打开生产者待写入的文件
  int fd = open(FILE_PATH, O_RDWR);
  read(fd, buff, sizeof(buff));
  printf("%s\n", buff);
  struct BufferPool *pool = (struct BufferPool *)arg;
  while (true) {
    if (pool->stat == 0) {
      pthread_mutex_lock(&mutex_pro);
      pool->stat = 1;
      if (pool->buffer[0] == '\0') {
        printf("Buffer is writing...\n");
        strncpy(pool->buffer, buff, sizeof(buff));
      }
      pthread_mutex_unlock(&mutex_pro);
      delay();
    } else {
      printf("Buffer is reading...\n");
      delay();
    }
  }
}

void Consumer(void *arg) {
  struct BufferPool *pool = (struct BufferPool *)arg;
  while (true) {
    //缓冲池正在被生成者使用，这时可以消费
    if (pool->stat == 1) {
      pthread_mutex_lock(&mutex_con);

    }
  }
}

int main() {
  pthread_t pro[PRODUCER_NUM], con[CONSUMER_NUM];
  int semid=semget(0x1111, 3, IPC_CREAT|0666);
  int pro_id = 99, shm_id;
  void *shm_ptr;
  pthread_mutex_init(&mutex_pro, NULL);
  pthread_mutex_init(&mutex_con, NULL);
  //创建共享内存
  shm_id = shmget((key_t)1234, sizeof(BufferPool), 0666 | IPC_CREAT);
  if (shm_id == -1) {
    exit(EXIT_FAILURE);
  }
  shm_ptr = shmat(shm_id, (void *)0, 0);
  if (shm_ptr == (void *)-1) {
    perror("shmat");
    exit(EXIT_FAILURE);
  }
  printf("Shared Mem is at:%p\n", shm_ptr);
  //创建各自进程
  memset(shm_ptr, 0, sizeof(BufferPool));
  for (int i = 0; i < PRODUCER_NUM; i++) {
    pthread_create(&pro[i], NULL, (void *(*)(void *))Producer, shm_ptr);
  }
  // for (int i = 0; i < CONSUMER_NUM; i++) {
  //   pthread_create(&con[i], NULL, (void *(*)(void *))Consumer, shm_ptr);
  // }
  for (int i = 0; i < CONSUMER_NUM; i++) {
    pthread_join(pro[i], NULL);
    pthread_join(con[i], NULL);
  }
}