#ifndef RINGDESCRIPTOR_H
#define RINGDESCRIPTOR_H
#include <cstdint>

//hmm just so we can return an empty descriptor ?
//what about alignement..
template<class T>
class RingDescriptor
{
public:
  RingDescriptor() {  flags&=~(0x1);}
  void  setValid(bool val)
  {
    flags&=~(0x1);
    if (val)
      flags|=0x1;
  }
  bool valid() {
    return (flags&0x1) == 1 ;
  }
  //could have had more!!
  void setId(uint8_t id)
  {
    flags |=(id<<24);
  }
  uint32_t id() { return (flags>>24)&0xFF; }

  void setRun(uint32_t run)
  {
    this->extra=run;
  }

  uint32_t run() { return this->extra; }

private:
  T *reference;
  uint32_t flags;
  uint32_t extra;
  uint8_t spacer[128-(sizeof(uint32_t)*2)-sizeof(T*)];
};
#endif // RINGDESCRIPTOR_H
