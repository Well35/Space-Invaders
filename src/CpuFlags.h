#pragma once

#include <stdint.h>

class CpuFlags {
public:
    bool z = 0; // zero flag
    bool s = 0; // sign flag
    bool p = 0; // parity flag 
    bool cy = 0;// carry flag
    bool ac = 0;// auxiliary carry flag
    //uint8_t pad = 0;
};