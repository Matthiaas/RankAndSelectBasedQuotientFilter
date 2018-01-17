
#ifndef AQM__H
#define AQM__H



#include <cinttypes>
#include "Element.h"

//Interface
class AQM {
   public:
      // pure virtual function
      virtual void insert(Element e) = 0;
      virtual uint64_t query(Element e) = 0;
      
};


#endif