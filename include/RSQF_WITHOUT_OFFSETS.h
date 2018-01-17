#ifndef RSQF_WITHOUT_OFFSETS__H
#define RSQF_WITHOUT_OFFSETS__H



#include "BitVectorOperations.h"
#include <cstdlib>
#include <inttypes.h>
#include <cmath>
#include <iostream>
#include <vector>

#include "Element.h"
#include "AQM.h"

namespace Filter{

class RSQF_WITHOUT_OFFSETS :public AQM{
public:
    RSQF_WITHOUT_OFFSETS(double false_positive_rate , int n);
    ~RSQF_WITHOUT_OFFSETS();

    void insert(Element e);
    uint64_t query(Element e);


private:
    size_t quotient;
    size_t remainder;
    size_t hashMask;

    std::vector<bool> occupieds;
    std::vector<bool> runends;
    std::vector<bool> remainders;
    std::vector<uint8_t> offset;
    
    size_t find_first_unused_slot(size_t x);
    

};

}
#endif