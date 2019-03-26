#include <iostream>
#include <thread>
#include <vector>
#include <map>

using namespace std;

#include "ringqueue.h"
#include "mutexring.h"
#include "atomicring.h"

#include "ringdescriptor.h"

RingQueue<RingDescriptor<int>,256> atomicRing;
AtomicRing<RingDescriptor<int>,256> atomicRing2;
MutexRing<RingDescriptor<int>,256> mutexRing;

uint64_t maximum=1000000;
uint64_t consumers=1;
uint64_t producers=1;

//#define STATS
#undef STATS

void consumer(RingInterface<RingDescriptor<int>,256> *ring,int id,uint64_t count)
{
  uint64_t received=0;
#ifdef STATS
  printf("Consumer expecting %d\n",maximum);
#endif
  std::map<int,int> counters;
  for (int i = 0; i < producers;i++)
    counters[i]=0;
  while(received < count)
  {
    auto desc=ring->getDescriptor();
    if (desc.valid())
    {
      counters[desc.id()]++;//desc.run();
      received++;
    }
  }
#ifdef STATS
  uint64_t total=0;
  for(int i = 0 ; i < counters.size();i++)
  {
    total+=counters[i];
    printf("received from[%d] : %d\n",i,counters[i]);
  }
  printf("Worker done recieved %u\n",total);
#endif
}

void producer(RingInterface<RingDescriptor<int>,256> *ring,int id,uint64_t count)
{

  RingDescriptor<int> r;
#ifdef STATS
  printf("I am a producer producing %d\n",max_prod);
#endif
  int produced=0;
  r.setValid(true);
  r.setId((uint8_t)id);
  r.setRun(0);

  while (produced < count)
  {
    if (ring->addDescriptor(r))
    {
      produced++;
      r.setRun(produced);
    }
  }
#ifdef STATS
  std::cout<<"Produced "<<produced<<"\n";
#endif
}

int test_ring(RingInterface<RingDescriptor<int>,256> *ring,std::string name,int prod,int cons,uint64_t consume)
{
  //we need to remove thread handling from biasing the test ?
  cout<<"starting ring test of type "<<name<<"\n";

  uint64_t produce_total=consume*cons;
  uint64_t prod_per=produce_total/prod;

  auto start = std::chrono::steady_clock::now();
  std::vector<std::thread> handles;
  int id=0;
  for (int i=0;i<producers;i++)
  {

    if (i+1 == producers)
    {
      prod_per=produce_total; //this makes the last one produce a few extra in cases where there is a division error
    }
    handles.emplace_back(std::thread(producer,ring,id,prod_per));
    produce_total-=prod_per;
    id++;
  }

  for (int i=0;i<consumers;i++)
  {
    handles.emplace_back(std::thread(consumer,ring,id,consume));
    id++;
  }

  std::cout<<"Waiting for threads to complete\n";
  for (auto &h : handles)
  {
    //is this were we are stuck ?
    h.join();
  }
  auto end = std::chrono::steady_clock::now();

  auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(end-start);

  std::cout<<name<<" Duration "<<dur.count()<<" ms\n";
  std::cout<<"Exchanged "<<(consumers*maximum*sizeof(RingDescriptor<int>))/(1024*1024)<<" Mbytes\n";
 // auto duration = std::chrono::duration::
}

int main(int argc, char *argv[])
{
  if (argc > 3)
  {
    string arg(argv[3]);
    maximum=std::stol(arg,nullptr,10);
 //   maximum=strtol(argv[1],NULL,10);
  }
  if (argc > 1)
  {
    string arg(argv[1]);
    producers=std::stol(arg,nullptr,10);
  }
  if (argc > 2)
  {
    string arg(argv[2]);
    consumers=std::stol(arg,nullptr,10);
  }
  printf("Stating test with %d procucers and %d consumers receiving %u\n",producers,consumers,maximum);

  test_ring(&atomicRing2,"AtomicRing2",producers,consumers,maximum);

  test_ring(&mutexRing,"mutexRing",producers,consumers,maximum);
  std::cout<<"Test completed\n";
  return 0;
}
