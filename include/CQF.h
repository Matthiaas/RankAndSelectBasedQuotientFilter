#ifndef CQF__H
#define CQF__H



#include <cstdlib>
#include <inttypes.h>
#include <cmath>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <climits>

#include "BitOperations.h"
#include "Element.h"
#include "Block.h"
#include "AQM.h"

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
#define FILTER_BROKEN "Filter may be broken!"

#define GET_BASE(n) ((n)== 0 ? (encodingBase+1):encodingBase)

// Possible returns of methode runContainsRemainder
#define ONE 1
#define TWO 2
#define THREE 3
#define COUNT 4

namespace CountingFilter{
    class CQF : public AQM{
    public:
        CQF(double false_positive_rate , int n);
        ~CQF();

        void insert(Element e);
        uint64_t query(Element e);

        int count = 0;


    private:

        uint64_t quotient;
        uint64_t remainder;

        uint64_t encodingBase;

        uint64_t hashMask;
        uint64_t remMask;

        uint64_t size;


        Block * blocks;
        uint64_t find_first_unused_slot( uint64_t x);
        uint64_t rankSelect( uint64_t h0);
        uint8_t runContainsRemainder(Block * blocks, uint64_t h0, uint64_t rem,uint64_t &pos );
        bool shiftRunendAndRemaindersByOne(uint64_t s, uint64_t slot);
        

    };
}

#endif
