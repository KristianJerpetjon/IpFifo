#ifndef MUTEXRING_H
#define MUTEXRING_H
#include <vector>
#include <mutex>

#include "ringinterface.h"


template<class T,int _size>
class MutexRing : public RingInterface<T,_size>
{
public:
  MutexRing() /*: RingInterface<T,_size>() we dont need this*/
  {
    this->head=this->tail=0;
  }

  bool addDescriptor(T &desc) final
  {
    //scoped lock
    std::lock_guard<std::mutex> lk(m);
    if (this->full(head,tail))
      return false;
    this->descriptors[head]=desc;
    head++;
    if (head == this->size)
      head=0;

    return true;
  }

  T getDescriptor() final
  {
    //scoped lock
    std::lock_guard<std::mutex> lk(m);
    if (this->empty(head,tail))
      return this->invalid;
    auto desc=this->descriptors[tail];
    tail++;
    if (tail == this->size)
      tail=0;
    return desc;
  }

private:
  std::mutex m;
  int head;
  int tail;
 /* int size=_size;
  RingDescriptor<T> descriptors[_size];
  RingDescriptor<T> invalid;*/
};

#endif // MUTEXRING_H
