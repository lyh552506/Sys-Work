#include "sys/sem.h"
#include <assert.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#define NUM 3
#define BUFFER_SIZE 27
#define BUFFER_NUM 5
#define FILE_PATH "./producerN.txt"
struct BufferPool {
  char buffer[BUFFER_NUM][BUFFER_SIZE];
  int stat
      [NUM]; // stat=0表示缓冲区未被生产者使用，可分配但不可消费；stat=1表示缓冲区已被生产者使用，不可分配但可消费
} BufferPool;
union semun {
  int val;
  struct semid_ds *buf;
  unsigned short *array;
};
void delay() {
  srand((unsigned)time(NULL));
  int t = rand() % 15000 + 9000;
  for (int i = 0; i < t; i++) {
    for (int j = 0; j < t; j++) {
      i = i + 1;
      i = i - 1;
    }
  }
}
int init_sem(int semid, int init_val) {
  union semun sem_union;
  sem_union.val = init_val;
  if (semctl(semid, 0, IPC_STAT, sem_union) == -1) {
    if (semctl(semid, 0, SETVAL, sem_union) == -1) {
      perror("semctl");
      return 1;
    }
  }
  return 0;
}

int del_sem(int semid) {
  union semun sem_union;
  if (semctl(semid, 0, IPC_RMID, sem_union) == -1) {
    // perror("semctl");
    return 1;
  }
  return 0;
}

int semaphore_p(int sem_id, short sem_no) {
  struct sembuf sem;
  sem.sem_num = sem_no;
  sem.sem_flg = SEM_UNDO;
  sem.sem_op = -1; //执行P操作
  if (semop(sem_id, &sem, 1) == -1) {
    // perror("P fail");
    return 0;
  }
  return 1;
}

int semaphore_v(int sem_id, short sem_no) {
  struct sembuf sem;
  sem.sem_num = sem_no;
  sem.sem_flg = SEM_UNDO;
  sem.sem_op = 1; //执行v操作
  if (semop(sem_id, &sem, 1) == -1) {
    // perror("V fail");
    return 0;
  }
  return 1;
}

int main() {
  int shm_id, index = 0, running;
  void *shm_ptr;
  char Product[27];
  struct BufferPool *pool;
  int visit_buff = semget(0x1111, 1, IPC_CREAT | 0666);
  int consumer_info = semget(0x1112, 1, IPC_CREAT | 0666);
  int productor_info = semget(0x1113, 1, IPC_CREAT | 0666);
  //初始化三个信号量
  init_sem(visit_buff, NUM);
  init_sem(consumer_info, 0);
  init_sem(productor_info, NUM);
  //初始化共享内存，首先尝试获得已存在的共享内存段
  shm_id = shmget((key_t)1246, sizeof(BufferPool), 0666);
  if (shm_id == -1)
    shm_id = shmget((key_t)1246, sizeof(BufferPool), 0666 | IPC_CREAT);
  if (shm_id == -1) {
    perror("shmget");
    return -1;
  }
  shm_ptr = shmat(shm_id, (void *)0, 0);
  memset(shm_ptr, 0, sizeof(BufferPool));
  pool = (struct BufferPool *)shm_ptr;
#ifdef DEBUG
  printf("Shared Mem is at:%p\n", pool);
#endif
  while (true) {
    //打开待读取文件
    int fd = open(FILE_PATH, O_RDWR);
    read(fd, Product, sizeof(Product));
    //进行信号量P操作
    semaphore_p(productor_info, 0);
    running = 1;
    while (running) {
      index %= BUFFER_NUM;
      semaphore_p(visit_buff, 0);
      //如果还未写入（生产者已经读出）
      if (pool->stat[index] == 0) {
        memcpy(pool->buffer[index], Product, sizeof(Product));
        printf("The Productor product: %s at buffer %d\n", Product, index);
        close(fd);
        semaphore_v(consumer_info, 0); //通知消费者可以进入
        pool->stat[index] = 1;
        running = 0;
        index++;
      } else if (pool->stat[index] == 1) {
        index++;
      }
      semaphore_v(visit_buff, 0); //释放资源
    }
    
  }
  shmdt(shm_ptr);
  //销毁信号量
  del_sem(visit_buff);
  del_sem(consumer_info);
  del_sem(productor_info);
}