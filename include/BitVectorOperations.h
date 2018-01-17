#ifndef BITVECTOR_OPERATIONS__H
#define BITVECTOR_OPERATIONS__H

#include <inttypes.h>
#include <cstdlib>
#include <vector>

#define BIT_MASK(n) (static_cast<uint64_t>(-1) >> (64-(n) ))

uint64_t select( std::vector<bool> &v , uint64_t i );

uint64_t rank( std::vector<bool> &v , uint64_t i );

uint64_t select( std::vector<bool> &v, uint64_t offset , uint64_t i );

uint64_t rank( std::vector<bool> &v, uint64_t offset  , uint64_t i );


bool bitVectorEquals(std::vector<bool> &a , uint64_t offseta, std::vector<bool> &b , uint64_t offsetb , uint64_t len );

void bitVectorSet(std::vector<bool> &dest , uint64_t offsetDest, std::vector<bool> &src , uint64_t offsetSrc , uint64_t len );

void writeValue(std::vector<bool> &dest ,uint64_t value , uint64_t len);

#endif