

#ifndef BITOPERATIONS__H
#define BITOPERATIONS__H

#include <inttypes.h>
#include <cstdlib>

#include "Properties.h"
#include "Block.h"

#define setBit(v , index) ((v) |= (1UL << (63-(index))))

#define unSetBit(v , index) ((v) &= ~(1UL << (63-(index))))

#define isBit(v , index) (((v) & (1UL<<(63 -(index)))) >>  (63-index))

#define BIT_MASK(n) (static_cast<uint64_t>(-1) >> (64-(n) ))

void shiftRight(uint64_t *v  , uint8_t from , uint8_t to , uint8_t by);

uint64_t rankOcc(Block * blocks , uint8_t fromBitIndex ,  uint64_t go );

uint64_t selectRunEnd(Block * blocks , uint8_t fromBitIndex , uint64_t go );


#endif
