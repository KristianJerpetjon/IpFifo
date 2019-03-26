#ifndef RINGQUEUE_H
#define RINGQUEUE_H

#include <atomic>

#include <stdio.h>

//if we insert pointers here.. then the data isnt copied but rather pointers are moved around "safely"


//See above!!

//this is more tricky!!
//make sure there is room
//then copy
//then insert if still room .. But there might not be anymore!!
//use the two heads and two tails ? or harden the memory access!!!!
//this poses a challange as we have to complete isertion before its read..

//Finally i remeber having the dual insert dual removal is the way to go
//so we insert at head then increment the real head after insertion ..
//but what if someone else does the same .. well simply put they cant!
//so what if we crash midst insert.. hmm this is more difficult than i remeber..
//but basically you cant increment "outer" "next" before "inner" next has catched up this does however cause a race condition where
//second is waiting for first and third is waiting for second etc.. this causes unacceptable amount of blocking..
//mutexing the whole thing is also an option but undesired!!

//alternative bit pattern style might be better ?

#include "ringinterface.h"

template<class T,int _size>
class RingQueue : public RingInterface<T,_size>
{
public:
  RingQueue() /*: Ring(size)*/
  {
    reset();
  }

  void reset()
  {
    readhead.store(0);
    readtail.store(0);
    writehead.store(0);
  }


  T getDescriptor() final
  {
    int localtail = readtail.load(std::memory_order_relaxed);

    //this should prevent us from doing ++ beyond tail
    if (this->empty(readhead.load(),localtail))
    {

      return this->invalid;
    }
    //if (readhead.load() == localtail) //has the tail catched up with the head so the ring is empty
    //  return invalid;

    int nexttail=localtail+1;


    //make a copy as we need to make sure we read before a consumer overwrites ??
    T desc=this->descriptors[localtail];

    //or we could do %size all over but what for ?
    if (nexttail == this->size)
      nexttail=0;

    //replace if failed return reference to empty descriptor.. this is to make the get completely non blocking
    if (!readtail.compare_exchange_weak(localtail,nexttail,std::memory_order_release,std::memory_order_relaxed))
      return this->invalid;

    //we know the update took place so we return head. (note that if we increment 10 and get return > 0 we can asume we got them all.
    return desc;
  }


  bool addDescriptor(T &desc) final
  {
    int wrhead=writehead.load(std::memory_order_relaxed);
    int nextwr=((wrhead+1)%this->size);

    if (this->full(wrhead,readtail.load()))
    {
      return false;
    }

    //could not get entry go out for a jiffie please come back later
    if (!writehead.compare_exchange_weak(wrhead,nextwr,std::memory_order_release,std::memory_order_relaxed))
    {
      return false;
    }

    //we got this far which means we can actually write in the location of wrhead uninterrupted
    this->descriptors[wrhead]=desc;
    //run catchup with readhed
    while(!readhead.compare_exchange_weak(wrhead,nextwr,std::memory_order_release,std::memory_order_relaxed));
    //report back that we succeded
    return true;
  }

private:
  //using alignas to space these apart will increase performance ?
  alignas(128) std::atomic<int> readhead;
  //uint8_t myspace[4096-sizeof(std::atomic<int>)];
  alignas(128) std::atomic<int> writehead;
 // uint8_t myspace1[4096-sizeof(std::atomic<int>)];
  alignas(128) std::atomic<int> readtail;
//  uint8_t myspace2[4096-sizeof(std::atomic<int>)];


};

#endif // RINGQUEUE_H
