#include "RSQF_WITHOUT_OFFSETS.h"


namespace Filter{

RSQF_WITHOUT_OFFSETS::RSQF_WITHOUT_OFFSETS( double false_positive_rate , int n){
    int p = (int) (log2(n/ false_positive_rate));
    if(p> 64){
        std::cout << "false positive_ rate was reduced!" << std::endl;
        p = 64;
    }
    hashMask = 0;
    for(int i = 0; i < p; i++){
        hashMask|= 1;
        hashMask <<= 1;
    }
    quotient =  p-3;
    remainder = (p-quotient);
    size_t slots = ((size_t) 2) << (quotient);



    occupieds.resize(slots);
    runends.resize(slots);
    remainders.resize(slots*remainder);
    offset.resize(slots);
   
}


RSQF_WITHOUT_OFFSETS::~RSQF_WITHOUT_OFFSETS(){

}

size_t RSQF_WITHOUT_OFFSETS::find_first_unused_slot(size_t x){
    size_t r = rank(occupieds , x);
    size_t s = select(runends , r);
    while(x <= s){
        x = s+1;
        r = rank(occupieds , x);
        s = select(runends , r);
    }
    return x;
}


void RSQF_WITHOUT_OFFSETS::insert(Element e){
    size_t h = e.hash1() & hashMask ;
    size_t h0 = h >> remainder;
    std::vector<bool> h1(remainder);
    writeValue(h1 , (h << quotient) >> quotient , remainder);  

    size_t r = rank(occupieds , h0);
    size_t s = select(runends , r);
    if(h0 > s){
        bitVectorSet(remainders , h0*remainder , h1 , 0 , remainder);
        runends[h0] = 1;
    }else{
        s = s+1;
        size_t n = find_first_unused_slot(s);
        while(n>s){
            bitVectorSet(remainders , n*remainder , remainders , (n-1)*remainder  , remainder);
            runends[n] =runends[n-1];
            n--;
        }
        bitVectorSet(remainders , s*remainder , h1 , 0 , remainder);
        if(occupieds[h0]){
            runends[s-1]= 0;
        }
        runends[s] = 1;
    }
    occupieds[h0] = 1;

}

uint64_t RSQF_WITHOUT_OFFSETS::query(Element e){
    size_t h = e.hash1() & hashMask;
    size_t b = h >> remainder; // h0
    if(!occupieds[b]){
        return false;
    }
    size_t t = rank(occupieds , b);
    size_t l = select(runends , t);
    std::vector<bool> v(remainder);
    writeValue(v , (h << quotient) >> quotient , remainder);  
    do{
        if(bitVectorEquals(remainders , l*remainder , v , 0 , remainder)){
            return true;
        }
        l--;
    }while(!(l < b || runends[l]));

    return false;

}

}