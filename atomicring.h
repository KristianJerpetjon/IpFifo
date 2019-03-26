#ifndef ATOMICRING_H
#define ATOMICRING_H

#include "ringinterface.h"

#include <atomic>

template<class T,int _size>
class AtomicRing : public RingInterface<T,_size>
{
public:
  AtomicRing() {
    readhead.store(0);
    writehead.store(0);
    readtail.store(0);
    writetail.store(0);
  }
  T getDescriptor() final
  {
    int rdtail = readtail.load();
    int nexttail=((rdtail+1)%this->size);
    //wr tail does not have to be the same as rdtail
    //is there a descriptor to read between tail and head
    if (this->empty(readhead.load(),rdtail))
    {
      return this->invalid;
    }

    //attempt to increment the read tail providing a future promise that we will increment the writetail
    //NB!!
    //if we fail between here and updating the write tail everything blows up..


    if (!readtail.compare_exchange_weak(rdtail,nexttail,std::memory_order_release,std::memory_order_relaxed))
    {
      return this->invalid;
    }

    //make a copy of the descriptor as we need to make sure we read before a producer overwrites the data
    auto desc=this->descriptors[rdtail];
    //printf("Reading %d desc valid=%d\n",readTail,desc.valid());
    //give in on the promise to update the write tail once it reaches the readtail
    //missing fucking semicolon!!
    auto curr=rdtail;

    while (!writetail.compare_exchange_weak(curr,nexttail,std::memory_order_release,std::memory_order_relaxed))
    {
      //we only want to update if current == rdtail;
      curr=rdtail;
    }
    //{
          //NBNBNBNB!!! ok so this has a fault as writetail gets updated on each run with actual value from another run..
      //to make sure the semicolon isnt missing
    //}

    //we know the update took place so we return head. (note that if we increment 10 and get return > 0 we can asume we got them all.
    return desc;
  }

  bool addDescriptor(T &desc) final
  {

    //first we read the tail. then we read the head
    int wrhead=writehead.load();

    //is the tail just behind the head abort
    if (this->full(wrhead,writetail.load()))
    {
     // printf("Full\n");
      return false;
    }

    int nextwr=((wrhead+1)%this->size);

    //could not get entry go out for a jiffie please come back later
    if (!writehead.compare_exchange_weak(wrhead,nextwr,std::memory_order_release,std::memory_order_relaxed))
    {
      return false;
    }

    //we got this far which means we can actually write in the location of wrhead uninterrupted
    this->descriptors[wrhead]=desc;

    auto curr=wrhead;
    //this must happen if and only if curr is what curr is..
    while(!readhead.compare_exchange_weak(curr,nextwr,std::memory_order_release,std::memory_order_relaxed))
    {
      curr=wrhead;
    }

    return true;
  }
private:
  //using alignas to space these apart will increase performance ?
  alignas(128) std::atomic<int> readhead;
  std::atomic<int> writehead;
  //uint8_t myspace1[4096-sizeof(std::atomic<int>)];
  alignas(128) std::atomic<int> readtail;
  std::atomic<int> writetail;
};

#endif // ATOMICRING_H
