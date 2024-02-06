#pragma once

#include <vector>
#include "CpuFlags.h"
#include <string>

// Intel 8080 CPU emulator
class Cpu {
public:
    Cpu();
    void emulate_cycle();
    void load_rom(std::string rom_name, uint16_t start_pos); // Will load the space invaders rom to beginning of memory

    void print_cpu_data();

    bool quit = false;
private:

    // registers
    uint8_t a = 0;
    uint8_t b = 0;
    uint8_t c = 0;
    uint8_t d = 0;
    uint8_t e = 0;
    uint8_t h = 0;
    uint8_t l = 0;

    uint16_t sp; // stack pointer
    uint16_t pc; // program counter

    std::vector<uint8_t> memory; // ram
    bool int_enable; // Interupt enable flag

    CpuFlags flags;
};

