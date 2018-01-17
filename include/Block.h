

#ifndef BLOCK__H
#define BLOCK__H

#include <inttypes.h>

#include "Properties.h"


struct Block{
    uint8_t offset:7;
    uint8_t used:1;
    uint64_t occupied;
    uint64_t runend;
    uint64_t remainder[REMAINDER_LEN];
}__attribute__((packed));






#endif