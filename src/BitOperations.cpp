


#include "BitOperations.h"

#if USE_POPCOUNT


//#include <immintrin.h>
// Compiler flags -mbmi -mbmi2
//inline uint64_t select( uint64_t v ,uint64_t i ){
//    return _tzcnt_u64(_pdep_u64(2l << i  , v ));
//}


inline uint64_t rank( uint64_t v , uint64_t i ){
    return __builtin_popcountll(v & ( BIT_MASK(i+1) << (63 -i)) );
}

#else

inline uint64_t rank( uint64_t v ,uint64_t i ){
	uint64_t res = 0;
    // TODO: < or <=
    for(uint64_t j = 0; j <= i; j++ ){
        res += isBit(v, j);
    }
    return res;
}

#endif

inline uint64_t select( uint64_t v , uint64_t &count ){
    for(uint64_t j = 0; j < 64; j++ ){
        count -= isBit(v, j);
        if(!count){
            return j;
        }
    }
    return 64;
}


void shiftRight(uint64_t *v  , uint8_t from , uint8_t to, uint8_t by){
	if(from > to) return;

	uint64_t mask = BIT_MASK( to -from +1);
	mask = mask <<(63-to);

	//Extract bits to shift
	uint64_t ext = *v & mask;
	ext = ext >> by;

	//Clear extracted bits
	*v = *v & ~mask;
	*v = *v & ~(mask >> by);

	//Set bits
	 *v = *v | ext;


}


uint64_t rankOcc(Block * blocks , uint8_t fromBitIndex , uint64_t go ){
	uint64_t res = 0;

	uint64_t i = 0;
	while(true){
		res+=rank ((blocks[i].occupied << fromBitIndex)
				|(blocks[i+1].occupied >> (64 - fromBitIndex))
					,go>64UL?64UL:go );
		i++;
		if(go < 64) break;
		go-=64;
	}


	return res;
}

uint64_t selectRunEnd(Block * blocks , uint8_t fromBitIndex , uint64_t count ){

	uint64_t res = 0;
	uint64_t i = 0;

	while(count != 0){
		res+=select ((blocks[i].runend << fromBitIndex)
				|(blocks[i+1].runend >> (64 - fromBitIndex))
					,count);
		i++;
	}
	return res;
}













