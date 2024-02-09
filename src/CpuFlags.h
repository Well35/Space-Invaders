#pragma once

#include <stdint.h>

class CpuFlags {
public:
    uint8_t z = 0; // zero flag
    uint8_t s = 0; // sign flag
    uint8_t p = 0; // parity flag 
    uint8_t cy = 0;// carry flag
    uint8_t ac = 0;// auxiliary carry flag
    //uint8_t pad = 0;
};