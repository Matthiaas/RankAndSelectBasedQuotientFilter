#ifndef BLOOM_FILTER__H
#define BLOOM_FILTER__H

#include <cmath>

#include "Element.h"
#include "AQM.h"

#define nthHash(hash1 , hash2 , n , size)((hash1 + n * hash2) % size)

//Get block index of given x
#define BLOCK_I(x) ((x)/64)
//Get bit index of given x in block BLOCK_I(x)
#define BIT_I(x) ((x)%64)


#define CHECK_BIT(v , index) (((v) >> (index))&1)
#define SET_BIT(v , index) ((v) |= (1UL << (index)))


namespace Filter{
    class BloomFilter : public AQM{
    public:
        BloomFilter( double false_positive_rate , long long n);
        ~BloomFilter();
        void insert(Element e);
        uint64_t query(Element e);

    private:
        uint64_t * bits;
        uint64_t size;
        uint32_t hashes;

    };

}

#endif