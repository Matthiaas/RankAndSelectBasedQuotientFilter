#include "CountingBloomFilter.h"

namespace CountingFilter{
CountingBloomFilter::CountingBloomFilter( double p ,long long n){
   
    double ln2 = log10(2.0);
    double m = (-n*log10(p)) / ( ln2*ln2);

#if BLOOM_COUNTING_BITS == 8
    counter = static_cast<uint8_t *> (calloc( (size_t)m , sizeof(uint8_t) ));;
#elif BLOOM_COUNTING_BITS == 16
    counter = static_cast<uint16_t *> (calloc( (size_t)m , sizeof(uint16_t) ));;;
#elif BLOOM_COUNTING_BITS == 32
    counter = static_cast<uint32_t *> (calloc( (size_t)m , sizeof(uint32_t) ));;;
#elif BLOOM_COUNTING_BITS == 64
    counter = static_cast<uint64_t *> (calloc( (size_t)m , sizeof(uint64_t) ));;;
#endif

    size =(uint64_t) m;
    this->hashes =(uint32_t) ceil (m/n *ln2);
}


CountingBloomFilter::~CountingBloomFilter(){
    free(counter);
}


void CountingBloomFilter::insert(Element e){
    for(int i = 0; i < hashes; i++){
        uint64_t hash = nthHash(e.hash1() , e.hash2() , i ,size);
        counter[hash]++;
    }
}


uint64_t CountingBloomFilter::query(Element e){
    uint64_t res = ULONG_MAX;
    for(int i = 0; i < hashes; i++){
        uint64_t hash = nthHash(e.hash1() , e.hash2() , i ,size);
        uint64_t curr = counter[hash];
        if(curr == 0) return 0;
        if(curr < res){
            res = curr;
        }
    }

    return res;
}
}