#include <iostream>
#include <chrono>
#include <mutex>
#include <thread>
#include <random>
#include <ctime>
#include <condition_variable>
using namespace std;
int myrand(int min, int max) {
  static mt19937 rnd(time(nullptr));
  return uniform_int_distribution<>(min,max)(rnd);
}
void phil(int ph, mutex& tryer, mutex& ma, mutex& mb, mutex& mo, unique_ptr<bool>& left, unique_ptr<bool>& ready, unique_ptr<std::condition_variable>& w1, unique_ptr<std::condition_variable>& w2, unique_ptr<int>& current, unique_ptr<bool[]>& readies) {
  for (;;) {
    int duration = myrand(200, 800);
    {
      
      lock_guard<mutex> gmo(mo);
      cout<<ph<<" thinks "<<duration<<"ms\n";
    }
    this_thread::sleep_for(chrono::milliseconds(duration));
    {
      lock_guard<mutex> gmo(mo);
      cout<<"\t\t"<<ph<<" is hungry\n";
    }
    {
      int hungryDuration = myrand(200, 800);
     this_thread::sleep_for(chrono::milliseconds(hungryDuration));
      {
      lock_guard<mutex> gmo(mo);
      cout<<"\t\t"<<ph<<" is ready to eat\n";
    }
    std::unique_lock<std::mutex> gma2(tryer);
    readies[ph] = true;
    while (!*(ready)) {
      for (int i = 0 ; i < 3; i++) {
        if (ph != *current && readies[i] != false) {
          (*w1).wait(gma2);
          if (*current != ph) {
            (*w1).notify_one();
            } else {
            break;
            }
        }
      }
      
    }
    lock_guard<mutex> gma(ma);
    
    
    lock_guard<mutex> gmb(mb);
    duration = myrand(200, 800);
    {
      lock_guard<mutex> gmo(mo);
      cout<<"\t\t\t\t"<<ph<<" eats "<<duration<<"ms\n";
    }
    (*left) = false;
    (*ready) = true;
    readies[ph] = false;
    (*w1).notify_one();
    *current = *current + 1 % 3;
    
    this_thread::sleep_for(chrono::milliseconds(duration));
    }
  }
}
int main() {
  cout<<"Fair dining Philosophers C++11 with Resource hierarchy\n";
  mutex m1, m2, m3;   // 3 forks are 3 mutexes
  mutex mo;           // for proper output
  mutex tryer;
  
  {
  
   unique_ptr<bool> value(new bool(true));
    unique_ptr<bool> ready(new bool(true));
   unique_ptr<std::condition_variable> w1(new std::condition_variable), w2(new std::condition_variable);
  unique_ptr<int> current(new int(0));
    unique_ptr<bool[]> readies(new bool[3]);

    for (int i = 0 ; i < 3; i ++) {
      readies[i] = true;
    }

  
  // 3 philosophers are 3 threads
  thread t1([&] {phil(0, tryer, m1, m2, mo, value, ready, w1, w2, current, readies);});
  thread t2([&] {phil(1, tryer, m2, m3, mo, value, ready, w1, w2, current, readies);});
  thread t3([&] {phil(2, tryer, m1, m3, mo, value, ready, w1, w2, current, readies);});
    thread t4([&] {phil(3, tryer, m1, m3, mo, value, ready, w1, w2, current, readies);});
    thread t5([&] {phil(5, tryer, m1, m3, mo, value, ready, w1, w2, current, readies);});
    thread t6([&] {phil(6, tryer, m1, m3, mo, value, ready, w1, w2, current, readies);});
    // Resource hierarchy
   // Fair scheduler
  t1.join();  // prevent threads from termination
  t2.join();
  t3.join();
  }
}
