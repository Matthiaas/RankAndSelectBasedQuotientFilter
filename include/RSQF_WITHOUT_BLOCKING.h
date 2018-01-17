#ifndef RSQF_WITHOUT_BLOCKING__H
#define RSQF_WITHOUT_BLOCKING__H



#include "BitVectorOperations.h"
#include <cstdlib>
#include <inttypes.h>
#include <cmath>
#include <iostream>
#include <vector>

#include "Properties.h"

#include "Element.h"
#include "AQM.h"

namespace Filter{

class RSQF_WITHOUT_BLOCKING : public AQM{ 
public:
    RSQF_WITHOUT_BLOCKING(double false_positive_rate , int n);
    ~RSQF_WITHOUT_BLOCKING();

    void insert(Element e);
    uint64_t query(Element e);

    int count = 0;


private:
    uint64_t quotient;
    uint64_t remainder;
    uint64_t hashMask;
    uint64_t remMask;

    std::vector<bool> occupieds;
    std::vector<bool> runends;
    std::vector<bool> remainders;
    std::vector<bool> offsetUsed;
    std::vector<uint8_t> offset;
    
    uint64_t find_first_unused_slot(uint64_t x);
    uint64_t rankSelect(uint64_t h0);
    

};

}
#endif