


#include "BitVectorOperations.h"


size_t select( std::vector<bool> &v , size_t i){
    return select(v , 0 , i);
}
size_t rank( std::vector<bool> &v , size_t i ){
	return rank(v , 0 , i);
}

size_t select( std::vector<bool> &v , size_t offset , size_t i){
    size_t count = i;
    for(size_t j = offset; j < v.size(); j++ ){
        count -= v[j];
        if(!count){
            return j - offset;
        }
    }
    //TODO: can this happen?
    return 0;    
}
size_t rank( std::vector<bool> &v, size_t offset , size_t i ){
    size_t res = 0;
    for(size_t j = offset; j <= i +offset ;j++ ){
        res += v[j];
    }
    
    return res;
}

bool bitVectorEquals(std::vector<bool> &a , size_t offseta, std::vector<bool> &b , size_t offsetb , size_t len ){
    for(size_t i = 0; i < len ; i++){
        if(a[offseta+i] != b[offsetb +i]){
            return false;
        }
    }
    return true;
}
void bitVectorSet(std::vector<bool> &dest , size_t offsetDest, std::vector<bool> &src , size_t offsetSrc , size_t len ){
    for(size_t i = 0; i < len ; i++){
        dest[offsetDest+i]= src[offsetSrc+i];
    }
}


void writeValue(std::vector<bool> &dest ,size_t value , size_t len){
    for(int i = len-1 , j = 0 ; i >= 0; j++ ,i--){
        dest[j] = 1 & (value >> i); 
    }
}


