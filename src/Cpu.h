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

    MachineIO io;
    uint8_t port = 0;
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

    
    bool int_enable = 0; // Interupt enable flag

    CpuFlags flags;
    

    int max_cycles_per_interupt = 16666;
    int current_cycles = 0;

    

    std::ofstream outs;

    // CPU instructions
    void NOP();
    void LXI(uint8_t& x, uint8_t& y, uint8_t byte1, uint8_t byte2);
    void STAXB();
    void STAXD();
    void INX(uint8_t& x, uint8_t& y);
    void INR(uint8_t& x);
    void INRM();
    void DEC(uint8_t& x);
    void DECM();
    void MVI(uint8_t& x, uint8_t& y);
    void DAD(uint8_t& x, uint8_t& y);
    void LDAX(uint8_t& x, uint8_t& y);
    void MOV(uint8_t& x, uint8_t& y);
    void MOVMR(uint8_t& x);
    void RET();
    void ORAR(uint8_t& x);
    void ORI(uint8_t& x);
    void ORAM();
    void ANAR(uint8_t& x);
    void ANI(uint8_t& x);
    void ANAM();
    void XRAR(uint8_t& x);
    void XRI(uint8_t& x);
    void XRAM();
    void CALLIF(uint8_t& x, uint8_t& y, bool con);
    void JUMPIF(uint8_t& x, uint8_t& y, bool con);
    void SBI(uint8_t& x);
    void ADDM();
    void CMP(uint8_t& x);
    void SUB(uint8_t& x);
    void ADD(uint8_t& x);
    void ADC(uint8_t& x);
};

