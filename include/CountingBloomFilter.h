#ifndef COUNTING_BLOOM_FILTER__H
#define COUNTING_BLOOM_FILTER__H

#include <cmath>
#include <climits>

#include "Element.h"
#include "AQM.h"
#include "Properties.h"

#define nthHash(hash1 , hash2 , n , size)((hash1 + n * hash2) % size)

namespace CountingFilter{
    class CountingBloomFilter : public AQM{
    public:
        CountingBloomFilter(double p ,long long n);
        ~CountingBloomFilter();
        void insert(Element e);
        uint64_t query(Element e);
    private:
    #if BLOOM_COUNTING_BITS == 8
        uint8_t * counter;
    #elif BLOOM_COUNTING_BITS == 16
        uint16_t * counter;
    #elif BLOOM_COUNTING_BITS == 32
        uint32_t * counter;
    #elif BLOOM_COUNTING_BITS == 64
        uint64_t * counter;
    #endif
        uint64_t size;
        uint32_t hashes;
    };
}
#endif