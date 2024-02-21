#include "Cpu.h"

#include <fstream>
#include <iostream>
#include <cstdint>
#include <SDL.h>


Cpu::Cpu() {
	memory = std::vector<uint8_t>(0xFFFF);

	pc = memory.at(0); // set program counter to start of memory

    // load the space invaders rom into memory
    load_rom("invaders", 0);

    //outs.open("log.txt");
}

void Cpu::emulate() {
    // This function will emulate 16666 cycles of instructions before causing the next automated interupt,
    //   which insures the graphics rendering and CPU execution are timed correctly
    // The alternative is to instead trigger an interupt everytime half the screen is rendered (similar to the how the machine did it in the 70's)
    while (current_cycles < max_cycles_per_interupt) {
        //outs << std::dec << "current: " << current_cycles << " max: " << max_cycles_per_interupt << std::endl;

        emulate_cycle();
        instructions_read++;
        
        if (quit) break;
    }
}

// Emulates a single instruction from the memory address of the PC
// TODO: Maybe seperate all opcodes into a different cpp file
void Cpu::emulate_cycle() {
    uint8_t opcode = memory.at(pc); // grab an opcode from pc location
    //outs << std::dec << "Instruction number: " << instructions_read << " opcode: " << std::hex << (opcode << 0)  << " Next byte: " << (memory.at(pc+1) << 0) << " Next byte: " << (memory.at(pc + 2) << 0) << std::endl;

    // Intel 8080 will sometime require the next two bytes in memory after the PC for an instruction
    // This means instructions will range from 1-3 bytes in length and the PC should be updated accordingly
    uint8_t byte1 = memory.at(pc + 1);
    uint8_t byte2 = memory.at(pc + 2);

    if (pc > 0xFFFF) {
        std::cout << "Went out of range in RAM! Oh no!" << std::endl;
        quit = true;
        return;
    }

    // Maybe refactor to something more organzied than a large switch statement
    // Instruction related functions should be put in a new file instead of making this one 900+ lines :o
    //   Most instructions that are resuable (ex. MOV) should be turned into functions that take registers as parameters
    switch (opcode) {
    case 0x00: NOP(); break; // NOP
    case 0x01: LXI(b, c, byte2, byte1); break; // LXI B,word
    case 0x02: STAXB(); break; // STAX B
    case 0x03: INX(b, c); break; //INX B
    case 0x04: INR(b); break; // INR B
    case 0x05: DEC(b); break; // DEC B
    case 0x06: MVI(b, byte1); break; // MVI B, D8
    case 0x07: { // RLC
        uint8_t x = a & 0x80;
        a = (a << 1) | (x >> 7);
        flags.cy = (x != 0);
        current_cycles += 4;
        break;
    }
    case 0x08: NOP(); break; // NOP
    case 0x09: DAD(b, c); break; // DAD B
    case 0x0A: LDAX(b, c); break; // LDAXB
    case 0x0B: { // DCX B
        uint32_t x = (b << 8) | c;
        x--;
        c = x & 0xFF;
        b = x >> 8;
        current_cycles += 5;
        break;
    }
    case 0x0C: INR(c); break; // INR C
    case 0x0D: DEC(c); break; // DEC C
    case 0x0E: MVI(c, byte1); break; // MVI C, D8
    case 0x0F: { //RRC
        uint8_t x = a & 0x01;
        a = (a >> 1) | (x << 7);
        flags.cy = x; // (TODO)
        current_cycles += 4;
        break;
    }
    case 0x10: NOP(); break; // NOP
    case 0x11: LXI(d, e, byte2, byte1); break; // LXI B,word
    case 0x12: STAXD(); break; // STAX D
    case 0x13: INX(d, e); break; //INX D
    case 0x14: INR(d); break; // INR D
    case 0x15: DEC(d); break; // DEC D
    case 0x16: MVI(d, byte1); break; // MVI D, D8
    case 0x18: NOP(); break; // NOP
    case 0x19: DAD(d, e); break; // DAD D
    case 0x1A: LDAX(d, e); break; // LDAXD
    case 0x1B: { // DCX D
        uint32_t x = (d << 8) | e;
        x--;
        e = x & 0xFF;
        d = x >> 8;
        current_cycles += 5;
        break;
    }
    case 0x1C: INR(e); break; // INR E
    case 0x1D: DEC(e); break; // DEC E
    case 0x1E: MVI(e, byte1); break; // MVI E, D8
    case 0x1f: { // RRA
        uint8_t x = a & 0x01;
        uint8_t y = flags.cy;
        a = ((a >> 1) | (y << 7));
        flags.cy = x;
        current_cycles += 4;
        break;
    }
    case 0x20: NOP(); break; // NOP
    case 0x21: LXI(h, l, byte2, byte1); break; // LXI H,word
    case 0x22: { // SHLD adr
        uint16_t x = (byte2 << 8 | byte1);
        memory.at(x) = l;
        memory.at(x + 1) = h;
        pc += 2;
        current_cycles += 16;
        break;
    }
    case 0x23: INX(h, l); break; //INX B
    case 0x24: INR(h); break; // INR H
    case 0x26: MVI(h, byte1); break; // MVI H, D8
    case 0x27: NOP(); break; // NOP
    case 0x28: NOP(); break; // NOP
    case 0x29: DAD(h, l); break; // DAD H
    case 0x2A: { // LHLD adr
        uint16_t x = ((byte2 << 8) | byte1);
        l = memory.at(x);
        h = memory.at(x + 1);
        pc += 2;
        current_cycles += 16;
        break;
    }
    case 0x2B: { // DCX H
        uint32_t x = (h << 8) | l;
        x--;
        l = x & 0xFF;
        h = (x >> 8) & 0xFF;
        current_cycles += 5;
        break;
    }
    case 0x2C: INR(l); break; // INR L
    case 0x2E: MVI(l, byte1); break; // MVI L, D8
    case 0x2F: // CMA
        a = ~a;
        current_cycles += 4;
        break;
    case 0x30: NOP(); break; // NOP
    case 0x31: // LXI SP,D16
        sp = (byte2 << 8) | byte1;
        pc += 2;
        //std::cout << "LXI SP,D16" << std::endl;
        current_cycles += 10;
        break;
    case 0x32: // STA adr
        memory.at(byte2 << 8 | byte1) = a;
        pc += 2;
        current_cycles += 13;
        //std::cout << "STA adr" << std::endl;
        break;
    case 0x34: INRM(); break; // INR M
    case 0x35: DECM(); break; // DEC M
    case 0x36: // MVI M,D8
        memory.at(h << 8 | l) = byte1;
        pc++;
        current_cycles += 10;
        break;
    case  0x37: // STC
        flags.cy = true;
        current_cycles += 4;
        break;
    case 0x38: NOP(); break; // NOP
    case 0x39: { // DAD SP
        uint16_t x = (h << 8) | l;
        uint32_t y = (uint32_t)sp + (uint32_t)x;
        h = (y >> 8) & 0xFF;
        l = y & 0xFF;
        flags.cy = y > 0xFFFF;
        current_cycles += 10;
        break;
    }
    case 0x3A: // LDA adr
        a = memory.at((byte2 << 8) | byte1);
        pc += 2;
        current_cycles += 13;
        //std::cout << "LDA adr" << std::endl;
        break;
    case 0x3C: INR(a); break; // INR A
    case 0x3D: DEC(a); break; // DEC A
    case 0x3E: MVI(a, byte1); break; // MVI A, D8
    case 0x41: MOV(b, c); break; // MOV B,C
    case 0x42: MOV(b, d); break; // MOV B,D
    case 0x43: MOV(b, e); break; // MOV B,E
    case 0x45: MOV(b, l); break; // MOV B,L
    case 0x46: MOVRM(b); break; // MOV B,M
    case 0x47: MOV(b, a); break; // MOV B,A
    case 0x48: MOV(c, b); break; // MOV C,B
    case 0x49: MOV(c, c); break; // MOV C,C
    case 0x4A: MOV(c, d); break; // MOV C,D
    case 0x4C: MOV(c, h); break; // MOV C,H
    case 0x4E: MOVRM(c); break; // MOV C,M
    case 0x4F: MOV(c, a); break; // MOV C,A
    case 0x51: MOV(d, c); break; // MOVE D,C
    case 0x54: MOV(d, h); break; // MOVE D,H
    case 0x56: MOVRM(d); break; // MOV D,M
    case 0x57: MOV(d, a); break; // MOVE D,A
    case 0x58: MOV(e, b); break; // MOVE E,B
    case 0x5E: MOVRM(e); break; // MOVE E,M
    case 0x5F: MOV(e, a); break; // MOV E,A
    case 0x60: MOV(h, b); break; // MOV H,B
    case 0x61: MOV(h, c); break; // MOV H,C
    case 0x65: MOV(h, l); break; // MOV H,L
    case 0x66: MOVRM(h); break; // MOV H,M
    case 0x67: MOV(h, a); break; // MOV H,A
    case 0x68: MOV(l, b); break; // MOV L,B
    case 0x69: MOV(l, c); break; // MOV L,C
    case 0x6B: MOV(l, e); break; // MOV L,E
    case 0x6D: MOV(l, l); break; // MOV L,L
    case 0x6E: MOVRM(l); break; // MOV L,M
    case 0x6F: MOV(l, a); break; // MOV L,A
    case 0x70: MOVMR(b); break; // MOV M,B
    case 0x71: MOVMR(c); break; // MOV M,C
    case 0x77: MOVMR(a); break; // MOV M,A
    case 0x78: MOV(a, b); break; // MOV A,B
    case 0x79: MOV(a, c); break; // MOV A,C
    case 0x7A: MOV(a, d); break; // MOV A,D
    case 0x7D: MOV(a, l); break; // MOV A,L
    case 0x7B: MOV(a, e); break; // MOV A,E
    case 0x7C: MOV(a, h); break; // MOV A,H
    case 0x7E: MOVRM(a); break; // MOV A,M
    case 0x7F: MOV(a, a); break; // MOV A,A
    case 0x80: ADD(b); break; // ADD B
    case 0x81: ADD(c); break; // ADD C
    case 0x82: ADD(d); break; // ADD D
    case 0x83: ADD(e); break; // ADD E
    case 0x85: ADD(l); break; // ADD L
    case 0x86: ADDM(); break; // ADD M
    case 0x8A: ADC(d); break; // ADC D
    case 0x90: SUB(b); break; // SUB B
    case 0x97: SUB(a); break; // SUB A
    case 0x9E: { // SBB M
        uint8_t x = memory.at(h << 8 | l);
        uint16_t r = a - x - flags.cy;
        sub_flags(a, x, flags.cy);
        a = r & 0xFF;
        current_cycles += 7;
        break;
    }
             /*....*/
    case 0xA0: ANAR(b); break; // ANA B
    case 0xA6: ANAM(); break; // ANA M
    case 0xA7: ANAR(a); break; // ANA A
    case 0xA8: XRAR(b); break; // XRA B
    case 0xAB: XRAR(e); break; // XRA E
    case 0xAF: XRAR(a); break; // XRA A
    case 0xB0: ORAR(b); break; // ORA B
    case 0xB4: ORAR(h); break; // ORA H
    case 0xB6: ORAM(); break; // ORA M
    case 0xB8: CMP(b); break; // CMP B
    case 0xBC: CMP(h); break; // CMP H
    case 0xBE: { // CMP M
        uint8_t x = a - memory.at(h << 8 | l);
        logic_flags(x);
        flags.cy = (memory.at(h << 8 | l) > a);
        current_cycles += 7;
        break;
    }
    case 0xC0: // RNZ address 
        if (!flags.z) {
            RET();
            current_cycles++;
        }
        else current_cycles += 5;
        break;
    case 0xC1: { // POP B
        c = memory.at(sp);
        b = memory.at(sp + 1);
        sp += 2;
        //std::cout << "POP B" << std::endl;
        current_cycles += 10;
        break;
    }
    case 0xC2: JUMPIF(byte2, byte1, !flags.z); break; // JNZ address 
    case 0xC3: JUMPIF(byte2, byte1, true); break; // JMP address
    case 0xC4: CALLIF(byte2, byte1, (!flags.z)); break; // CNZ address 
    case 0xC5: { // PUSH B
        memory.at(sp - 1) = b;
        memory.at(sp - 2) = c;
        sp -= 2;
        //current_cycles += 11; // TODO: maybe doesnt have a cycle cost
        break;
    }
    case 0xC6: { // ADI byte
        uint16_t answer = (uint16_t)a + (uint16_t)memory.at(pc + 1);
        add_flags(a, memory.at(pc + 1), 0);
        a = answer & 0xFF;
        pc++;
        //std::cout << "ADI byte" << std::endl;
        current_cycles += 7;
        break;
    }
    case 0xC8: // RZ 
        if (flags.z) {
            RET();
            current_cycles++;
        }
        else {
            current_cycles += 5;
        }
        break;
    case 0xCA: JUMPIF(byte2, byte1, flags.z); break; // JZ address 
    case 0xCD: { // CALL address
        uint16_t ret = pc + 3;

        memory.at(sp - 1) = ((ret >> 8) & 0xff);
        memory.at(sp - 2) = (ret & 0xff);
        sp -= 2;
        pc = (memory.at(pc + 2) << 8) | memory.at(pc + 1);
        pc--;

        current_cycles += 17;
        break;
    }
    case 0xC9: RET(); break; // RET address
    case 0xCC: CALLIF(byte2, byte1, flags.z); break; // CZ address 
    case 0xD0: // RNC 
        if (!flags.cy) {
            RET();
            current_cycles++;

        }
        else {
            current_cycles += 5;
        }
        break;
    case 0xD1: { // POP D
        e = memory.at(sp);
        d = memory.at(sp + 1);
        sp += 2;
        //std::cout << "POP D" << std::endl;
        current_cycles += 10;
        break;
    }
    case 0xD2: JUMPIF(byte2, byte1, !flags.cy); break; // JNC address 
    case 0xD3: // OUT (TODO)
        io.write(memory.at(pc + 1), a);
        pc++;
        current_cycles += 10;
        //std::cout << "OUT" << std::endl;
        break;
    case 0xD4: CALLIF(byte2, byte1, !flags.cy); break; // CNC address 
    case 0xD5: // PUSH D
        memory.at(sp - 2) = e;
        memory.at(sp - 1) = d;
        sp -= 2;
        current_cycles += 10; //(TODO)
        break;
    case 0xD6: { // SUI
        uint8_t t = -(static_cast<uint8_t>(memory.at(pc + 1)));
        uint16_t x = a + t;
        sub_flags(a, memory.at(pc + 1), 0);
        a = x & 0xFF;
        current_cycles += 7;
        pc++;
        break;
    }
    case 0xD8: // RC 
        if (flags.cy) {
            RET();
            current_cycles++;
        }
        else {
            current_cycles += 5;
        }
        break;
    case 0xD9: NOP(); break; // NOP
    case 0xDA: JUMPIF(byte2, byte1, flags.cy); break; // JC address 
    case 0xDB: // IN (TODO)
        a = io.read(byte1);
        pc++;
        current_cycles += 10;
        //std::cout << "IN" << std::endl;
        break;
    case 0xDE: SBI(byte1); break; // SBI
    case 0xE0: // RPO
        if (!flags.p) {
            RET();
            current_cycles++;
        }
        else {
            current_cycles += 5;
        }
        break;
    case 0xE1: { // POP H
        l = memory.at(sp);
        h = memory.at(sp + 1);
        sp += 2;
        //std::cout << "POP H" << std::endl;
        current_cycles += 10;
        break;
    }
    case 0xE2: JUMPIF(byte2, byte1, !flags.p); break; // JPO address
    case 0xE3: { // XTHL
        uint8_t x = memory.at(sp);
        uint8_t y = memory.at(sp + 1);

        memory.at(sp) = l;
        memory.at(sp + 1) = h;
        l = x;
        h = y;

        current_cycles += 18;
        break;
    }
    case 0xE5: // PUSH H
        memory.at(sp - 2) = l;
        memory.at(sp - 1) = h;
        sp -= 2;
        //current_cycles += 11; //(TODO)
        break;
    case 0xE6: ANI(byte1); break; // ANI byte

    case 0xEB: { // XCHG
        uint8_t x = h;
        h = d;
        d = x;

        x = l;
        l = e;
        e = x;
        //std::cout << "XCHG" << std::endl;
        current_cycles += 4;
        break;
    }
    case 0xE9: // PCHL
        pc = (h << 8) | l;
        pc--;
        current_cycles += 5;
        break;
    case 0xEC: CALLIF(byte2, byte1, flags.p); break; // CPE address 
    case 0xEE: XRI(byte1); break; // XRI D8
    case 0xf1: { //POP PSW    
        a = memory.at(sp + 1);
        uint8_t psw = memory.at(sp);
        flags.cy = ((psw & 0x01) != 0);   // bit 0
        flags.p = ((psw & 0x04) != 0);   // bit 2
        flags.ac = ((psw & 0x10) != 0);   // bit 4
        flags.z = ((psw & 0x40) != 0);   // bit 6
        flags.s = ((psw & 0x80) != 0);   // bit 7
        sp += 2;
        current_cycles += 10;
        //std::cout << "POP PSW CALLED" << std::endl;
        //quit = true;
        break;
    }
    case 0xf5: { // PUSH PSW
        memory.at(sp - 1) = a;
        uint8_t psw = 0x0;
        if (flags.cy) psw |= 0x01;
        psw |= 0x02;
        if (flags.p) psw |= 0x04;
        if (flags.ac) psw |= 0x10;
        if (flags.z) psw |= 0x40;
        if (flags.s) psw |= 0x80;
        memory.at(sp - 2) = psw;
        sp -= 2;
        current_cycles += 11;
        //std::cout <<  "PUSH PSW CALLED" << std::endl;
        //quit = true;
        break;
    }
    case 0xF6: ORI(byte1); break; // ORI D8
    case 0xFA: JUMPIF(byte2, byte1, flags.s); break;// JM address 
    case 0xFB: // EI
        int_enable = true;
        current_cycles += 4;
        //std::cout << "EI" << std::endl;
        break;
    case 0xFC: CALLIF(byte2, byte1, flags.s); break; // CM address 
    case 0xFE: { // CPI byte
        uint8_t x = a - byte1;
        inr_flags(x);
        flags.cy = byte1 > a;
        pc++;
        current_cycles += 7;
        break;
    }
    default:
        // If an opcode is yet to be implemented, this case will be called and kill the program
        std::cout << std::hex << "Unimplemented opcode: " << (opcode << 0) << std::endl;
        quit = true;
        break;
    }
    pc++;
}

void Cpu::load_rom(std::string rom_name, uint16_t start_pos) {
    std::ifstream file(rom_name, std::ios::binary | std::ios::ate);
    if (file.is_open()) {
        // Get size of file and allocate a buffer to hold the contents
        std::streampos size = file.tellg();
        char* buffer = new char[size];

        // Go back to the beginning of the file and fill the buffer
        file.seekg(0, std::ios::beg);
        file.read(buffer, size);
        file.close();

        //std::cout << std::hex << "Loading rom: " << rom_name << " from " << (start_pos << 0) << " to " << ((start_pos + size) << 0) << std::endl;
        for (uint32_t i = 0; i < size; i++) {
            //std::cout << std::hex << (memory.at(i) << 0) << std::endl;
            memory.at(i+start_pos) = buffer[i];
        }

        delete[] buffer;
    }
}

void Cpu::input() {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            quit = true;
        }
        if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
                case SDLK_c:        io.port1 |= 0x01; break;
                case SDLK_RETURN:   io.port1 |= 0x04; break;
                case SDLK_SPACE:    io.port1 |= 0x10; break;
                case SDLK_LEFT:     io.port1 |= 0x20; break;
                case SDLK_RIGHT:    io.port1 |= 0x40; break;
            }
        }
        if (event.type == SDL_KEYUP) {
            switch (event.key.keysym.sym) {
                case SDLK_c:        io.port1 &= 0xfe; break;
                case SDLK_RETURN:   io.port1 &= 0xfb; break;
                case SDLK_SPACE:    io.port1 &= 0xef; break;
                case SDLK_LEFT:     io.port1 &= 0xdf; break;
                case SDLK_RIGHT:    io.port1 &= 0xbf; break;
            }
        }
    }
}

void Cpu::interupt(uint16_t intp) {
    current_cycles = 0;
    if (int_enable) { 
        //std::cout << "Interupted!" << std::endl;
        switch (intp) {
            case 0xCF:
                memory.at(sp - 1) = (pc >> 8) & 0xFF;
                memory.at(sp - 2) = pc & 0xFF;
                pc = 0x1 * 8;
                sp -= 2;
                current_cycles += 11;
                break;
            case 0xD7:
                memory.at(sp - 1) = (pc >> 8) & 0xFF;
                memory.at(sp - 2) = pc & 0xFF;
                pc = 0x2 * 8;
                sp -= 2;
                current_cycles += 11;
                break;
            default:
                break;
        }
        int_enable = false;
        //outs << std::dec << "Instruction number: " << instructions_read << " opcode: " << std::hex << (intp << 0) << std::endl;
        instructions_read++;
    }
}

void Cpu::print_cpu_data() {
    outs << std::hex << " a: " << (a << 0) << " b: " << (b << 0) << " c: " << (c << 0) << " d: " << (d << 0) << " e: " << (e << 0) << " h: " << (h << 0) << " l: " << (l << 0) << std::endl;
    outs << "flags: " << flags.z << flags.s << flags.p << flags.cy << flags.ac << std::endl;
    outs << "sp: " << std::hex << (sp << 0) << " pc: " << (pc << 0) << std::endl;
}

bool Cpu::parity(uint8_t byte) {
    bool parity_table[] = {
      1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
      0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
      0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
      1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
      0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
      1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
      1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
      0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
      0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
      1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
      1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
      0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
      1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
      0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
      0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
      1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1
    };
    return parity_table[byte];
}

// Handles flags for addition instructions
void Cpu::add_flags(uint8_t x, uint8_t y, uint8_t carry) {
    uint16_t sum = (uint16_t)x + (uint16_t)y + carry;
    flags.z = ((uint8_t)(x + y + carry) == 0);
    flags.s = ((sum & 0x80) == 0x80);
    flags.p = parity(sum & 0xFF);
    flags.cy = sum > 0xFF;
}

// Handles flags for subtraction instructions
void Cpu::sub_flags(uint8_t x, uint8_t y, uint8_t carry) {
    uint16_t result = (uint16_t)x - (uint16_t)y - carry;
    flags.z = (result == 0);
    flags.s = ((result & 0x80) == 0x80);
    flags.p = parity(uint8_t(x - y - carry));
    flags.ac = ~(x ^ y ^ result) & 0x10;
}

// Handles flags for increasing a register by 1 instructions
void Cpu::inr_flags(uint8_t x) {
    flags.z = (x == 0);
    flags.s = (x & 0x80) != 0;
    flags.p = parity(x);
}

// Handles flags related to bit logic
void Cpu::logic_flags(uint8_t x) {
    flags.z = (x == 0);
    flags.s = ((x & 0x80) != 0);
    flags.p = parity(x);
    flags.cy = 0;
}

// Below are all instructions for the Intel 8080 and can be called by using registers/next few bytes in memory as arguments

void Cpu::NOP() {
    current_cycles += 4;
}

void Cpu::LXI(uint8_t& x, uint8_t& y, uint8_t byte1, uint8_t byte2) {
    x = byte1;
    y = byte2;
    pc += 2;
    current_cycles += 10;
}

void Cpu::STAXB() {
    memory.at((b << 8) | c) = a;
    current_cycles += 7;
}

void Cpu::STAXD() {
    memory.at((d << 8) | e) = a;
    current_cycles += 7;
}

void Cpu::INX(uint8_t& x, uint8_t& y) {
    uint16_t n = (x << 8) | y;
    n++;
    x = (uint8_t)((n >> 8) & 0xff);
    y = (uint8_t)(n & 0xff);
    current_cycles += 5;
}

void Cpu::INR(uint8_t& x) {
    x++;
    inr_flags(x);
    current_cycles += 5;
}

void Cpu::INRM() {
    uint8_t x = memory.at((h << 8) | l);
    x++;
    memory.at((h << 8) | l) = x;
    inr_flags(x);
    current_cycles += 10;
}

void Cpu::DEC(uint8_t& x) {
    x--;
    inr_flags(x);
    current_cycles += 5;
}

void Cpu::DECM() {
    uint8_t x = memory.at((h << 8) | l);
    x--;
    memory.at((h << 8) | l) = x;
    inr_flags(x);
    current_cycles += 10;
}

void Cpu::MVI(uint8_t& x, uint8_t& y) {
    x = y;
    pc++;
    current_cycles += 7;
}

void Cpu::DAD(uint8_t& x, uint8_t& y) {
    uint16_t xy = (x << 8) | y;
    uint16_t hl = (h << 8) | l;
    uint16_t sum = hl + xy;

    h = ((sum >> 8) & 0xFF);
    l = sum & 0xFF;

    flags.cy = (sum > 0xFFFF);
    current_cycles += 10;
}

void Cpu::LDAX(uint8_t& x, uint8_t& y) {
    a = memory.at((x << 8) | y);
    current_cycles += 7;
}

void Cpu::MOV(uint8_t& x, uint8_t& y) {
    x = y;
    current_cycles += 5;
}

void Cpu::MOVMR(uint8_t& x) {
    memory.at((h << 8) | l) = x;
    current_cycles += 7;
}

void Cpu::MOVRM(uint8_t& x) {
    x = memory.at((h << 8) | l);
    current_cycles += 7;
}

void Cpu::RET() {
    pc = memory.at(sp) | ((memory.at(sp + 1) << 8) & 0xFF00);
    sp += 2;
    pc--;
    current_cycles += 10;
}

void Cpu::ORAR(uint8_t& x) {
    a |= x;
    logic_flags(a);
    current_cycles += 4;
}

void Cpu::ORI(uint8_t& x) {
    a |= x;
    logic_flags(a);
    pc++;
    current_cycles += 7;
}

void Cpu::ORAM() {
    a |= memory.at((h << 8) | l);
    logic_flags(a);
    current_cycles += 7;
}

void Cpu::ANAR(uint8_t& x) {
    a &= x;
    logic_flags(a);
    current_cycles += 4;
}

void Cpu::ANI(uint8_t& x) {
    a &= x;
    logic_flags(a);
    pc++;
    current_cycles += 7;
}

void Cpu::ANAM() {
    a &= memory.at((h << 8) | l);
    logic_flags(a);
    current_cycles += 4;
}

void Cpu::XRAR(uint8_t& x) {
    a ^= x;
    logic_flags(a);
    current_cycles += 4;
}

void Cpu::XRI(uint8_t& x) {
    a ^= x;
    logic_flags(a);
    pc++;
    current_cycles += 7;
}

void Cpu::XRAM() {
    a ^= memory.at((h << 8) | l);
    logic_flags(a);
    current_cycles += 4;
}

void Cpu::CALLIF(uint8_t& x, uint8_t& y, bool con) {
    if (con) {
        uint16_t ret = pc + 3;

        memory.at(sp - 1) = (ret >> 8) & 0xff;
        memory.at(sp - 2) = (ret & 0xff);
        sp -= 2;
        pc = (x << 8) | y;
        pc--;
        current_cycles += 17;
    }
    else {
        pc += 2;
        current_cycles += 11;
    }
}

void Cpu::JUMPIF(uint8_t& x, uint8_t& y, bool con) {
    if (con) {
        pc = (x << 8) | y;
        pc--;
    }
    else {
        pc += 2;
    }

    current_cycles += 10;
}

void Cpu::SBI(uint8_t& x) {
    uint16_t y = a - (x + flags.cy);
    flags.cy = (y & 0x100) == 0;
    inr_flags(y);
    a = y & 0xFF;
    current_cycles += 7;
    pc++;
}

void Cpu::ADDM() {
    uint8_t x = memory.at((h << 8) | l);
    add_flags(a, x, 0);
    a += x;
    current_cycles += 7;
}

void Cpu::CMP(uint8_t& x) {
    uint8_t y = a - x;
    logic_flags(y);
    flags.cy = y > a;
    current_cycles += 4;
}

void Cpu::SUB(uint8_t& x) {
        uint16_t r = a - x;
        sub_flags(a, x, 0);
        a = r & 0xFF;
        current_cycles += 4;
}

void Cpu::ADD(uint8_t& x) {
        uint16_t answer = a + x;
        add_flags(a, x, 0);
        a = answer & 0xFF;
        current_cycles += 4;
}

void Cpu::ADC(uint8_t& x) {
    uint16_t answer = a + x + flags.cy;
    add_flags(a, x, flags.cy);
    a = answer & 0xFF;
    current_cycles += 4;
}
