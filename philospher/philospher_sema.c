//============================利用信号量semaphore机制=====================================
//=======================================================================================
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define NUM 3
sem_t chopstick[NUM];
int phi[NUM]={1,2,3};

void delay() {
  srand((unsigned)time(NULL));
  int t = rand() % 30000 + 10000;
  for (int i = 0; i < t; i++) {
    for (int j = 0; j < t; j++) {
      i = i + 1;
      i = i - 1;
    }
  }
}
void think() { delay(); }

void eat(int x) {
  sem_wait(&chopstick[x]);
  delay();
  sem_wait(&chopstick[(x + 1) % NUM]);
  printf("%d is eating now\n", x);
  delay();
  sem_post(&chopstick[x]);
  sem_post(&chopstick[(x + 1) % NUM]);
}
void* philospher(void* args) {
  int x=*(int*)args;
  while (1) {
    printf("I am %d phi\n", x);
    printf("%d am going to think\n", x);
    think();
    printf("%d am hungry\n", x);
    eat(x);
  }
}

int main() {
  printf("利用信号量semaphore机制\n");
  pthread_t philos[NUM];
  for (int i = 0; i < NUM; i++)
    sem_init(&chopstick[i], 0, 1);
  for(int i=0;i<NUM;i++)
   pthread_create(&philos[i], NULL, philospher, &phi[i]);
  for (int i = 0; i < NUM; i++){
    pthread_join(philos[i], NULL);
    sem_destroy(&chopstick[i]);

  }
}