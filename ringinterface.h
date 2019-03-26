#ifndef RINGINTERFACE_H
#define RINGINTERFACE_H
#include "ringdescriptor.h"

template<class T,int _size>
class RingInterface
{
public:
  inline bool full(int head,int tail) noexcept
  {
    //does this cause an undesired optimalisation ?
    if ((((head+1)%size) == tail))
      return true;
    return false;
  }

  inline bool empty(int head,int tail) noexcept
  {
    if (head == tail)
      return true;
    return false;
  }

  virtual T getDescriptor()=0;
  virtual bool addDescriptor(T &desc)=0;

protected:
  //data pointers
  T descriptors[_size];
  //actual data cointainer
 // T data[_size];
  T invalid;

  int size=_size;
};

#endif // RINGINTERFACE_H
