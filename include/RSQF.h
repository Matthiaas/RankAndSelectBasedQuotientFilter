
#ifndef RSQF__H
#define RSQF__H



#include <cstdlib>
#include <inttypes.h>
#include <cmath>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <climits>


#include "AQM.h"
#include "BitOperations.h"
#include "Element.h"
#include "Block.h"

//Get block index of given x
#define BLOCK_I(x) ((x)/64)
//Get bit index of given x in block BLOCK_I(x)
#define BIT_I(x) ((x)%64)


//Print error
#define ERROR(x) std::cerr << (x) << std::endl

//Error codes
#define UNDF_BEHV "Undefined behavior"
#define OFFSET_TO_SMALL "Offsets to small"
#define FILTER_TO_SMALL "Insert not possible! (Filter to small)"
#define FALSE_POSITIVE_REDUCE "false positive rate was reduced!"

namespace Filter{
    class RSQF : public AQM{
    public:
        RSQF(double false_positive_rate , int n);
        ~RSQF();

        void insert(Element e);
        uint64_t query(Element e);

        int count = 0;


    private:
        uint64_t quotient;
        uint64_t remainder;
        uint64_t hashMask;
        uint64_t remMask;

        uint64_t size;


        Block * blocks;
        uint64_t find_first_unsed_slot( uint64_t x);
        uint64_t rankSelect( uint64_t h0);
        bool shiftRunendAndRemaindersByOne(uint64_t s);
        

    };
}

#endif