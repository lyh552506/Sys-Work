//============================利用互斥量mutex加锁机制=====================================
//=======================================================================================
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define NUM 5
pthread_mutex_t mut[NUM];
// 哲学家自己的编号
int philos[NUM] = {1, 2, 3, 4, 5};
// template <typename Derived>
// class Base{
//   public:
//    void Print()const{
//     static_cast<Derived*>(this)->Print();
//    }
// };

// class D1:public Base<D1>{
//   void Print(){
//     std::cout<<"i am D1"
//     <<std::endl;
//   }
// };

// class D2:public Base<D2>{
//   void Print(){
//     std::cout<<"i am D2"
//     <<std::endl;
//   }
// };

// int main(){
//   D1 d1;
//   D2 d2;
// }
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
void init_lock() {
  for (int i = 0; i < NUM; i++)
    pthread_mutex_init(&mut[i], NULL);
}

//这里采用非阻塞的trylock
void eat_with_trylock(int x) {
  //奇数哲学家
  if (x % 2 == 1) {
    while (pthread_mutex_trylock(&mut[x]) != 0)
      delay();
    while (pthread_mutex_trylock(&mut[(x + 1) % NUM]) != 0)
      delay();
    printf("%d philospher is eating now\n", x);
    pthread_mutex_unlock(&mut[x]);
    pthread_mutex_unlock(&mut[(x + 1) % NUM]);
  } else {
    while (pthread_mutex_trylock(&mut[(x + 1) % NUM]) != 0)
      delay();
    while (pthread_mutex_trylock(&mut[x]) != 0)
      delay();
    printf("%d philospher is eating now\n", x);
    delay();
    pthread_mutex_unlock(&mut[x]);
    pthread_mutex_unlock(&mut[(x + 1) % NUM]);
  }
}
void think() { delay(); }

//会造成死锁
void eat_with_lock(int x) {
  pthread_mutex_lock(&mut[x]);
  pthread_mutex_lock(&mut[(x + 1) % NUM]);
  printf("%d philospher is eating now\n", x);
  delay();
  pthread_mutex_unlock(&mut[x]);
  pthread_mutex_unlock(&mut[(x + 1) % NUM]);
}

void eat_change_lock(int x) {
  if (x % 2 == 0) {
    pthread_mutex_lock(&mut[x]);
    pthread_mutex_lock(&mut[(x + 1) % NUM]);
    printf("%d philospher is eating now\n", x);
  } else {
    pthread_mutex_lock(&mut[(x+1)%NUM]);
    pthread_mutex_lock(&mut[x]);
    printf("%d philospher is eating now\n", x);
  }
  delay();
  pthread_mutex_unlock(&mut[x]);
  pthread_mutex_unlock(&mut[(x + 1) % NUM]);
}

//让权等待方式解决死锁
void eat(int x) {
  int delay_flag = 0;
  while (pthread_mutex_trylock(&mut[x]) != 0) {
    delay();
  }
  while (pthread_mutex_trylock(&mut[(x + 1) % NUM]) != 0) {
    delay();
    delay_flag++;
    if (delay_flag > 120) {
      pthread_mutex_unlock(&mut[x]);
      printf("no chopstick can provide\n");
      return;
    }
  }
  printf("%d philospher is eating now\n", x);
  pthread_mutex_lock(&mut[x]);
  pthread_mutex_lock(&mut[(x + 1) % 5]);
}

//规定第i个哲学家的左边筷子序号是i，右边是i+1
//并规定奇数哲学家用左边的筷子，偶数哲学家用右边的筷子
void *philospher(void *arg) {
  int p = *(int *)(arg);
  while (1) {
    printf("I am %d phi\n", p);
    printf("%d am going to think\n", p);
    think();
    printf("%d am hungry\n", p);
    //eat_with_trylock(p);
    //eat_with_lock(p);
    //eat(p);
    eat_change_lock(p);
  }
}
int main() {
  pthread_t p[NUM];
  init_lock();
  for (int i = 0; i < 5; i++) {
    int err =
        pthread_create(&p[i], NULL, (void *(*)(void *))philospher, &philos[i]);
    if (err != 0) {
      printf("can not creat thread because : %s\n", strerror(err));
      return 1;
    }
  }
  for (int i = 0; i < 5; i++) {
    pthread_join(p[i], NULL);
    pthread_mutex_destroy(&mut[i]);
  }
}