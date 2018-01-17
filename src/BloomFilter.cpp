#include "BloomFilter.h"


namespace Filter{

BloomFilter::BloomFilter( double p ,long long n){
   
    double ln2 = log10(2.0);
    double m = (-n*log10(p)) / ( ln2*ln2);
    bits = static_cast<uint64_t *> (calloc( (size_t)m/64 , sizeof(uint64_t) ));
    size =(uint64_t) m;
    this->hashes =(uint32_t) ceil (m/n *ln2);
}
BloomFilter::~BloomFilter(){
    free(bits);
}


void BloomFilter::insert(Element e){
    for(int i = 0; i < hashes; i++){
        uint64_t hash = nthHash(e.hash1() , e.hash2() , i ,size);
        SET_BIT( bits[BLOCK_I(hash)] , BIT_I(hash) );
    }
}


uint64_t BloomFilter::query(Element e){
    for(int i = 0; i < hashes; i++){
        uint64_t hash = nthHash(e.hash1() , e.hash2() , i ,size);
        if(!CHECK_BIT( bits[BLOCK_I(hash)] , BIT_I(hash) ))
            return false;
    }

    return true;
}


}