#include <iostream>
#include <list>
class Stack;

class Observer {
public:
  Stack *stack;
  Observer(Stack *stack);
  virtual ~Observer();
  virtual void update(int) = 0;
};

class Monitor : Observer {
public:
  Monitor(Stack *stack) : Observer(stack) {}
  void print(int v) const { std::cout << "Monitor: " << v << std::endl; }
  void update(int v) override { print(v); }
};

class Screen : Observer {
public:
  Screen(Stack *stack) : Observer(stack) {}
  void display(int v) const { std::cout << "Screen: " << v << std::endl; }
  void update(int v) override { display(v); }
};

class Stack {
public:
  int price = 0;
  std::list<Observer *> ObserveList;
  void attach(Observer *observe) { ObserveList.push_back(observe); }
  void dettach(Observer *observe) { ObserveList.remove(observe); }
  void show(int v) {
    for (auto observe : ObserveList)
      observe->update(v);
  }
  void setPrice(int v) {
    price = v;
    show(price);
  }
};

Observer::Observer(Stack *stack) : stack(stack) { stack->attach(this); }
Observer::~Observer() { stack->dettach(this); }

int main() {
  Stack stack;
  Monitor monitor(&stack);
  Screen screen(&stack);
  stack.setPrice(5);
}