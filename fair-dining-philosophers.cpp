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
void phil(int ph, mutex& tryer, mutex& ma, mutex& mb, mutex& mo, unique_ptr<bool>& left, unique_ptr<bool>& ready, unique_ptr<std::condition_variable>& w1, unique_ptr<std::condition_variable>& w2) {
  for (;;) {  // prevent thread from termination
    int duration = myrand(200, 800);
    {
      // Block { } limits scope of lock
      lock_guard<mutex> gmo(mo);
      cout<<ph<<" thinks "<<duration<<"ms\n";
    }
    this_thread::sleep_for(chrono::milliseconds(duration));
    {
      lock_guard<mutex> gmo(mo);
      cout<<"\t\t"<<ph<<" is hungry\n";
    }
    {
    std::unique_lock<std::mutex> gma2(tryer);
    
    while (!(*left) && !*(ready)) (*w1).wait(gma2);
    }
    lock_guard<mutex> gma(ma);
    
    this_thread::sleep_for(chrono::milliseconds(400));
    
    lock_guard<mutex> gmb(mb);
    duration = myrand(200, 800);
    {
      lock_guard<mutex> gmo(mo);
      cout<<"\t\t\t\t"<<ph<<" eats "<<duration<<"ms\n";
    }
    (*left) = false;
    (*ready) = true;
    
    (*w1).notify_one();
    
    this_thread::sleep_for(chrono::milliseconds(duration));
  }
}
int main() {
  cout<<"dining Philosophers C++11 with Resource hierarchy\n";
  mutex m1, m2, m3;   // 3 forks are 3 mutexes
  mutex mo;           // for proper output
  mutex tryer;
  
  {
  
   unique_ptr<bool> value(new bool(true));
    unique_ptr<bool> ready(new bool(true));
   unique_ptr<std::condition_variable> w1(new std::condition_variable), w2(new std::condition_variable);

  
  // 3 philosophers are 3 threads
  thread t1([&] {phil(1, tryer, m1, m2, mo, value, ready, w1, w2);});
  thread t2([&] {phil(2, tryer, m2, m3, mo, value, ready, w1, w2);});
  thread t3([&] {phil(3, tryer, m1, m3, mo, value, ready, w1, w2);});  // Resource hierarchy
   // Fair scheduler
  t1.join();  // prevent threads from termination
  t2.join();
  t3.join();
  }
}
