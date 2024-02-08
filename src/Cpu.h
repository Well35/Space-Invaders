#pragma once

#include <vector>
#include "CpuFlags.h"
#include "MachineIO.h"
#include <string>
#include <fstream>

// Intel 8080 CPU emulator
class Cpu {
public:
    Cpu();
    void emulate();
    void emulate_cycle();
    void load_rom(std::string rom_name, uint16_t start_pos); // Will load the space invaders rom to beginning of memory
    void input();
    void interupt(uint16_t intp);

    void print_cpu_data();

    bool quit = false;
    int instructions_read = 0;

    std::vector<uint8_t> memory; // ram
private:

    bool parity(uint8_t byte);
    void add_flags(uint8_t x, uint8_t y, uint8_t carry);
    void sub_flags(uint8_t x, uint8_t y, uint8_t carry);
    void inr_flags(uint8_t x);
    void logic_flags(uint8_t x);

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

    
    bool int_enable; // Interupt enable flag

    CpuFlags flags;
    MachineIO io;

    int max_cycles_per_interupt = 16666;
    int current_cycles = 0;

    std::ofstream outs;
};

