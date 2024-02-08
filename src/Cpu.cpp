#include "Cpu.h"

#include <fstream>
#include <iostream>
#include <cstdint>
#include <SDL.h>


Cpu::Cpu() {
	memory = std::vector<uint8_t>(64000); // 64KB of memory

	pc = memory.at(0); // set program counter to start of memory

    // load all 4 roms to their specified location in memory
    load_rom("invaders", 0);
    //load_rom("invaders.h", 0);
    //load_rom("invaders.e", 0x1800);
    //load_rom("invaders.f", 0x1000);
    //load_rom("invaders.g", 0x0800);

    outs.open("log.txt");
}

void Cpu::emulate() {
    current_cycles = 0;

    while (current_cycles < max_cycles_per_interupt) {
        //std::cout << std::dec << "current: " << current_cycles << " max: " << max_cycles_per_interupt << std::endl;
        //if (instructions_read == 48000) quit = true;
        emulate_cycle();
        instructions_read++;
        
        if (quit) break;
    }
}

// TODO: currently bug with storing values in the wrong places
//          grabbing data from the HL address is grabbing 0 instead of the requested data from ram
void Cpu::emulate_cycle() {
	uint8_t opcode = memory.at(pc); // grab an opcode from pc location
    //if (instructions_read > 1500000) std::cout << std::dec << "Instruction number: " << instructions_read << " opcode: " << std::hex << (opcode << 0)  << " Next byte: " << (memory.at(pc+1) << 0) << " Next byte: " << (memory.at(pc + 2) << 0) << std::endl;

    if (pc > 0x4000) {
        std::cout << "Went out of range in RAM! Oh no!" << std::endl;
        quit = true;
        return;
    }
    
    //std::cout << std::hex << "pc: " << (pc << 0) << " opcode: " << (opcode << 0) << " Desc: ";

	// Maybe refactor to something more organzied than a large switch statement
	switch (opcode) {
		case 0x00: // NOP 
            //std::cout << "NOP" << std::endl;
            current_cycles += 4;
			break;
		case 0x01: // LXI B,word B <- byte 3, C <- byte 2
			c = memory.at(pc + 1);
			b = memory.at(pc + 2);
            //std::cout << "LXI B,word" << std::endl;
            pc += 2;
            current_cycles += 10;
			break;
        case 0x02: // STAX B
            memory.at(b << 8 | c) = a;
            current_cycles += 7;
            //std::cout << "STAX B" << std::endl;
            break;
        case 0x03: // INX B
            c++;
            if (c == 0) b++;
            current_cycles += 5;
            break;
        case 0x04: // INR B
            b++;
            inr_flags(b);
            current_cycles += 5;
            break;
        case 0x05: // DEC B
            b--;
            inr_flags(b);
            current_cycles += 5;
            //TODO: ac flag
            //std::cout << "DEC B " << flags.s << std::endl;
            break;
        case 0x06: // MVI B, D8
            b = memory.at(pc + 1);
            pc++;
            //std::cout << "MVI B, D8" << std::endl;    
            current_cycles += 7;
            break;
        case 0x07: { // RLC
            uint8_t x = a & 0x80;
            a = (a << 1) | (x >> 7);
            flags.cy = (x != 0);
            current_cycles += 4;
            break;
        }
        case 0x08: // NOP
            current_cycles += 4;
            break;
        case 0x09: { // DAD B
            uint32_t x = (h << 8) | l;
            uint32_t y = (b << 8) | c;
            x += y;
            flags.cy = (x > 0xffff);
            l = x & 0xFF;
            h = (x >> 8) & 0xFF;
            current_cycles += 10;
            //std::cout << std::hex << "DAD D x: " << (x << 0) << " h: " << (h << 0) << " l: " << (l << 0) << std::endl;
            break;
        }
        case 0x0A: // LDAXB
            a = memory.at(b << 8 | c);
            //std::cout << "LDAXB " << std::hex << (a << 0) << std::endl;
            current_cycles += 7;
            break;
        case 0x0B: { // DCX B
            uint32_t x = (b << 8) | c;
            x--;
            c = x & 0xFF;
            b = x >> 8;
            current_cycles += 5;
            break;
        }
        case 0x0C: // INR C
            c++;
            inr_flags(c);
            current_cycles += 5;
            break;
        case 0x0D: // DEC C
            c--;
            inr_flags(c);
            //TODO: ac flag
            //std::cout << "DEC C " << flags.s << std::endl;
            current_cycles += 5;
            break;
        case 0x0E: // MVI C,D8
            c = memory.at(pc + 1);
            pc++;
            //std::cout << "MVI C,D8" << std::endl;
            current_cycles += 7;
            break;
        case 0x0F: { //RRC
            uint8_t x = a & 0x01;
            a = (a >> 1) | (x << 7);
            flags.cy = x; // (TODO)
            current_cycles += 4;
            break;
        }
        case 0x10: // NOP 
            current_cycles += 4;
            break;
        case 0x11: // LXI D,D16
            e = memory.at(pc + 1);
            d = memory.at(pc + 2);
            pc += 2;
            //std::cout << "LXI D,D16" << std::endl;
            current_cycles += 10;
            break;
        case 0x13: { // INX D
            e++;
            if (e == 0) d++;
            current_cycles += 5;
            //std::cout << std::hex << "INX D x: " << (x << 0) << " d: " << (h << 0) << " e: " << (l << 0) << std::endl;
            break;
        }
        case 0x14: // INR D
            d++;
            inr_flags(d);
            current_cycles += 5;
            break;
        case 0x15: // DEC D
            d--;
            inr_flags(d);
            current_cycles += 5;
            break;
        case 0x16: // MVI D,D8
            d = memory.at(pc + 1);
            pc++;
            current_cycles += 7;
            break;
        case 0x18: // NOP
            current_cycles += 4;
            break;
        case 0x19: { // DAD D
            uint32_t x = (h << 8) | l;
            uint32_t y = (d << 8) | e;
            x += y;
            flags.cy = (x > 0xffff);
            l = x & 0xFF;
            h = x >> 8;
            current_cycles += 10;
            //std::cout << std::hex << "DAD D x: " << (x << 0) << " h: " << (h << 0) << " l: " << (l << 0) << std::endl;
            break;
        }
        case 0x1A: // LDAXD
            // not sure if this is right
            a = memory.at(d << 8 | e);
            //std::cout << "LDAXD " << std::hex << (a << 0) << std::endl;
            current_cycles += 7;
            break;
        case 0x1C: // INR E
            e++;
            inr_flags(e);
            current_cycles += 5;
            break;
        case 0x1D: // DEC E
            e--;
            inr_flags(e);
            current_cycles += 5;
            break;
        case 0x1E: // MVI E,D8
            e = memory.at(pc + 1);
            pc++;
            current_cycles += 7;
            break;
        case 0x1f: { // RRA
            uint8_t x = a & 0x01;
            uint8_t y = flags.cy;
            a = (a >> 1 | y << 7);
            flags.cy = (x != 0);
            current_cycles += 4;
            break;
        }
        case 0x20:
            current_cycles += 4;
            break;
        case 0x21: // LXI H,D16
            l = memory.at(pc + 1);
            h = memory.at(pc + 2);
            pc += 2;
            //std::cout << "LXI H,D16" << std::endl;
            current_cycles += 10;
            break;
        case 0x22: { // SHLD adr
            uint16_t x = (pc + 2 | pc + 1);
            memory.at(x) = l;
            memory.at(x + 1) = h;
            pc += 2;
            current_cycles += 16;
            break;
        }
        case 0x23: { // INX H
            l++;
            if (l == 0) h++;
            //std::cout << std::hex << "INX H x: " << (x << 0) << " h: " << (h << 0) << " l: " << (l << 0) << std::endl;
            current_cycles += 5;
            break;
        }
        case 0x24: // INR H
            h++;
            inr_flags(h);
            current_cycles += 5;
            break;
        case 0x26: // MVI H,D8
            h = memory.at(pc + 1);
            pc++;
            //std::cout << "MVI H,D8" << std::endl;
            current_cycles += 7;
            break;
        case 0x28: // NOP 
            current_cycles += 4;
            break;
        case 0x29: { // DAD H
            uint32_t x = (h << 8) | l;
            x += x;
            flags.cy = (x > 0xffff);
            l = x & 0xFF;
            h = x >> 8;
            current_cycles += 10;
            //std::cout << std::hex << "DAD H x: " << (x << 0) << " h: " << (h << 0) << " l: " << (l << 0) << std::endl;
            break;
        }
        case 0x2A: { // LHLD adr
            uint16_t address = (memory.at(pc + 2) << 8) | memory.at(pc + 1);
            l = memory.at(address);
            h = memory.at(address + 1);
            pc += 2;
            current_cycles += 16;
            break;
        }
        case 0x2B: { // DCX H
            uint32_t x = (h << 8) | l;
            x--;
            l = x & 0xFF;
            h = x >> 8;
            current_cycles += 5;
            break;
        }
        case 0x2E: // MVI L, D8
            l = memory.at(pc + 1);
            pc++;
            current_cycles += 7;
            break;
        /*....*/
        case 0x2F: // CMA
            a = ~a;
            //std::cout << "CMA" << std::endl;
            current_cycles += 4;
            break;
        case 0x30: // NOP
            current_cycles += 4;
            break;
        case 0x31: // LXI SP,D16
            sp = (memory.at(pc + 2) << 8) | memory.at(pc + 1);
            pc += 2;
            //std::cout << "LXI SP,D16" << std::endl;
            current_cycles += 10;
            break;
        case 0x32: // STA adr
            memory.at(memory.at(pc + 2) << 8 | memory.at(pc + 1)) = a;
            pc += 2;
            current_cycles += 13;
            //std::cout << "STA adr" << std::endl;
            break;
        case 0x34: { // INR M
            uint8_t x = memory.at(h << 8 | l);
            x++;
            memory.at(h << 8 | l) = x;
            inr_flags(x);
            current_cycles += 10;
            break;
        }
        case 0x35: { // DEC M
            uint16_t x = memory.at(h << 8 | l);
            x--;
            memory.at(h << 8 | l) = x;
            inr_flags(x);
            //TODO: ac
            current_cycles += 10;
            break;
        }
        case 0x36: // MVI M,D8
            memory.at(h << 8 | l) = memory.at(pc+1);
            pc++;
            //std::cout << "MVI M,D8 " << std::endl;
            current_cycles += 10;
            break;
        case  0x37: // STC
            flags.cy = true;
            current_cycles += 4;
            break;
        case 0x38: // NOP 
            current_cycles += 4;
            break;
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
            a = memory.at(memory.at(pc+2) << 8 | memory.at(pc+1));
            pc += 2;
            current_cycles += 13;
            //std::cout << "LDA adr" << std::endl;
            break;
        case 0x3C: // INR A
            a++;
            inr_flags(a);
            current_cycles += 5;
            break;
        case 0x3D: // DEC A
            a--;
            inr_flags(a);
            //TODO: ac flag
            //std::cout << "DEC C " << flags.s << std::endl;
            current_cycles += 5;
            break;
        case 0x3E: // MVI A,D8
            a = memory.at(pc + 1);
            pc++;
            //std::cout << "MVI A,D8" << std::endl;
            current_cycles += 7;
            break;
        case 0x41: // MOV B,C
            b = c;
            //std::cout << "MOV B,C" << std::endl;
            current_cycles += 5;
            break;
        case 0x42: // MOV B,D
            b = d;
            //std::cout << "MOV B,D" << std::endl;
            current_cycles += 5;
            break;
        case 0x43: // MOV B,E
            b = e;
            //std::cout << "MOV B,E" << std::endl;
            current_cycles += 5;
            break;
        case 0x44: // MOV B,H
            b = h;
            current_cycles += 5;
            break;
        case 0x45: // MOV B,L
            b = l;
            current_cycles += 5;
            break;
        case 0x46: // MOVE B,M
            b = memory.at(h << 8 | l);
            current_cycles += 7;
            break;
        case 0x48: // MOV C,B
            c = b;
            current_cycles += 5;
            break;
        case 0x49: // MOV C,C
            c = c;
            current_cycles += 5;
            break;
        case 0x4A: // MOV C,D
            c = d;
            current_cycles += 5;
            break;
        case 0x4C: // MOV C,H
            c = h;
            current_cycles += 5;
            break;
        case 0x4E: // MOVE C,M
            c = memory.at(h << 8 | l);
            current_cycles += 7;
            break;
        case 0x4F: // MOV C,A
            c = a;
            current_cycles += 5;
            break;
        case 0x51: // MOVE D,C
            d = c;
            current_cycles += 5;
            break;
        case 0x54: // MOVE D,H
            d = h;
            current_cycles += 5;
            break;
        case 0x56: // MOVE D,M
            d = memory.at(h << 8 | l);
            //std::cout << "MOV D,M" << std::endl;
            current_cycles += 7;
            break;
        case 0x57: // MOVE D,A
            d = a;
            current_cycles += 5;
            break;
        case 0x58: // MOVE E,B
            e = b;
            current_cycles += 5;
            break;
        case 0x5E: // MOVE E,M
            e = memory.at(h << 8 | l);
            //std::cout << "MOV E,M" << std::endl;
            current_cycles += 7;
            break;
        case 0x5F: // MOV E,A
            e = a;
            current_cycles += 5;
            break;
        case 0x60: // MOV H,B
            h = b;
            current_cycles += 5;
            break;
        case 0x61: // MOV H,C
            h = c;
            current_cycles += 5;
            break;
        case 0x65: // MOV H,L
            h = l;
            current_cycles += 5;
            break;
        case 0x66: // MOV H,M
            h = memory.at(h << 8 | l);
            //std::cout << "MOV H,M" << std::endl;
            current_cycles += 7;
            break;
        case 0x67: // MOV H,A
            h = a;
            //std::cout << "MOV H,A" << std::endl;
            current_cycles += 5;
            break;
        case 0x68: // MOV L,B
            l = b;
            current_cycles += 5;
            break;
        case 0x6B: // MOV L,E
            l = e;
            current_cycles += 5;
            break;
        case 0x6D: // MOV L,L
            l = l;
            current_cycles += 5;
            break;
        case 0x6E: // MOV L,M
            l = memory.at(h << 8 | l);
            current_cycles += 7;
            break;
        case 0x6F: // MOV L,A
            l = a;
            //std::cout << "MOV L,A" << std::endl;
            current_cycles += 5;
            break;
        case 0x70: // MOV M,B
            memory.at(h << 8 | l) = b;
            current_cycles += 7;
            break;
        case 0x77: // MOV M,A
            // TODO: check correctness
            memory.at(h << 8 | l) = a;
            //std::cout << "MOV M,A " << std::hex << (memory.at(h << 8 | l) << 0) << std::endl;
            current_cycles += 7;
            break;
        case 0x78: // MOV A,B
            a = b;
            current_cycles += 5;
            break;
        case 0x79: // MOV A,C
            a = c;
            current_cycles += 5;
            break;
        case 0x7A: // MOV A,D
            a = d;
            //std::cout << "MOV A,D" << std::endl;
            current_cycles += 5;
            break;
        case 0x7D: // MOV A,L
            a = l;
            current_cycles += 5;
            break;
        case 0x7B: // MOV A,E
            a = e;
            //std::cout << "MOV A,E" << std::endl;
            current_cycles += 5;
            break;
        case 0x7C: // MOV A,H
            a = h;
            //std::cout << "MOV A,H" <<  std::endl;
            current_cycles += 5;
            break;
        /*....*/
        case 0x7E: // MOVE A,M
            //TODO: might have to fix this
            // storing to that memory location might have a bug
            a = memory.at(h << 8 | l);
            //std::cout << "MOV A,M" << std::endl;
            current_cycles += 7;
            break;
        case 0x7F: // MOV C,C
            c = c;
            current_cycles += 5;
            break;
        case 0x80: { // ADD B
            uint16_t answer = (uint16_t)a + (uint16_t)b;
            add_flags(a, b, 0);
            a = answer & 0xFF; // back to 8 bit
            //std::cout << "ADD B" << std::endl;
            current_cycles += 4;
            break;
        }
        case 0x81: { // ADD C
            uint16_t answer = (uint16_t)a + (uint16_t)c;
            flags.z = ((answer & 0xff) == 0);
            add_flags(a, c, 0);
            a = answer & 0xff; // back to 8 bit
            //std::cout << "ADD C" << std::endl;
            current_cycles += 4;
            break;
        }
        case 0x82: { // ADD D
            uint16_t answer = (uint16_t)a + (uint16_t)d;
            add_flags(a, d, 0);
            a = answer & 0xff;
            current_cycles += 4;
            break;
        }
        case 0x90: { // SUB B
            uint16_t r = a - b;
            sub_flags(a, b, 0);
            a = r & 0xFF;
            current_cycles += 4;
            break;
        }
        case 0x9E: { // SBB M
            uint8_t x = memory.at(h << 8 | l);
            uint16_t r = a - x - flags.cy;
            sub_flags(a, x, flags.cy);
            a = r & 0xFF;
            current_cycles += 7;
            break;
        }
        /*....*/
        case 0xA0: // ANA B
            a &= b;
            logic_flags(a);
            //std::cout << "ANA B" << std::endl;
            current_cycles += 4;
            break;
        case 0xA6: { // ANA M
            uint8_t x = memory.at(h << 8 | l);
            a &= x;
            logic_flags(a);
            current_cycles += 4;
            break;
        }
        case 0xA7: // ANA A
            a = a & a;
            logic_flags(a);
            //std::cout << "ANA A" << std::endl;
            current_cycles += 4;
            break;
        case 0xA8: // XRA B
            a ^= b;
            logic_flags(a);
            current_cycles += 4;
        case 0xAB: // XRA E
            a ^= e;
            logic_flags(a);
            current_cycles += 4;
        case 0xAF: // XRA A
            a ^= a;
            logic_flags(a);
            //TODO: ac flag
            //std::cout << "XRA A" << std::endl;
            current_cycles += 4;
            break;
        case 0xB0: { // ORA B
            a |= b;
            logic_flags(a);
            current_cycles += 4;
            break;
        }
        case 0xB4: { // ORA H
            a |= h;
            logic_flags(a);
            current_cycles += 4;
            break;
        }
        case 0xB6: { // ORA M
            uint8_t x = memory.at(h << 8 | l);
            a |= x;
            logic_flags(a);
            current_cycles += 7;
            break;
        }
        case 0xBE: { // CMP M
            uint8_t x = a - memory.at(h << 8 | l);
            logic_flags(x);
            flags.cy = (memory.at(h << 8 | l) > a);
            current_cycles += 7;
            break;
        }
        case 0xC0: // RNZ address 
            if (flags.z == 0) {
                pc = memory.at(sp) | ((memory.at(sp + 1) << 8) & 0xFF00);
                sp += 2;
                current_cycles += 11;
                pc--;
            }
            else current_cycles += 5;
            break;
        case 0xC1: { // POP B
            c = memory.at(sp);
            b = memory.at(sp+1);
            sp += 2;
            //std::cout << "POP B" << std::endl;
            current_cycles += 10;
            break;
        }
        case 0xC2: // JNZ address 
            if (flags.z == 0) {
                pc = (memory.at(pc + 2) << 8) | memory.at(pc + 1); // turn next 2 bytes into 16 bit number and store in the PC
                pc--;
            }
            else {
                pc += 2;
            }
            //std::cout << "JNZ address" << std::endl;
            current_cycles += 10;
            break;
        case 0xC3: // JMP address
            pc = (memory.at(pc + 2) << 8) | memory.at(pc + 1);
            pc--;
            //std::cout << "JMP address " << std::hex << (pc + 1 << 0) << std::endl;
            current_cycles += 10;
            break;
        case 0xC4: // CNZ address 
            //TODO: Might need to fix
            if (flags.cy == 0) {
                uint16_t ret = pc + 3;

                memory.at(sp - 1) = (ret >> 8) & 0xff;
                memory.at(sp - 2) = (ret & 0xff);
                sp -= 2;

                pc = (memory.at(pc + 2) << 8) | memory.at(pc + 1);
                pc--;

                current_cycles += 17;
            }
            else {
                pc += 2;
                current_cycles += 11;
            }
            break;
        case 0xC5: { // PUSH B
            memory.at(sp-1) = b;
            memory.at(sp-2) = c;
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
            if (flags.z != 0) {
                pc = memory.at(sp) | ((memory.at(sp + 1) << 8) & 0xFF00);
                sp += 2;
                current_cycles += 11;
                pc--;
            }
            else {
                current_cycles += 5;
            }
            break;
        case 0xCA: // JZ address 
            if (flags.z != 0) {
                pc = (memory.at(pc + 2) << 8) | memory.at(pc + 1); // turn next 2 bytes into 16 bit number and store in the PC
                pc--;
            }
            else
                // branch not taken    
            pc += 2;
            //std::cout << "JZ address" << std::endl;
            current_cycles += 10;
            break;
        case 0xCD: { // CALL address
            uint16_t ret = pc + 3;

            memory.at(sp - 1) = ((ret >> 8) & 0xff); // store the return location into 2 bytes of memory
            memory.at(sp - 2) = (ret & 0xff);
            sp -= 2;

            pc = (memory.at(pc + 2) << 8) | memory.at(pc + 1); // move pc to the call address
            pc--;  // revert the pc++

            //std::cout << "CALL address" << std::endl;
            current_cycles += 17;
            break;
        }
        case 0xC9: // RET address
            pc = memory.at(sp) | ((memory.at(sp + 1) << 8) & 0xFF00);
            sp += 2;
            pc--;
            //std::cout << "RET address" << std::endl;
            current_cycles += 10;
            break;
        case 0xCC: // CZ address 
            if (flags.z != 0) {
                uint16_t ret = pc + 3;

                memory.at(sp - 1) = (ret >> 8) & 0xff;
                memory.at(sp - 2) = (ret & 0xff);
                sp -= 2;

                pc = (memory.at(pc + 2) << 8) | memory.at(pc + 1);
                pc--;

                current_cycles += 17;
            }
            else {
                pc += 2;
                current_cycles += 11;
            }
            break;
        case 0xD0: // RNC 
            if (flags.cy == 0) {
                pc = memory.at(sp) | ((memory.at(sp + 1) << 8) & 0xFF00);
                sp += 2;
                current_cycles += 11;
                pc--;
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
        case 0xD2: // JNC address 
            if (flags.cy == 0) {
                pc = (memory.at(pc + 2) << 8) | memory.at(pc + 1); // turn next 2 bytes into 16 bit number and store in the PC
                pc--;
            }
            else
                // branch not taken    
                pc += 2;
            //std::cout << "JNC address" << std::endl;
            current_cycles += 10;
            break;
        case 0xD3: // OUT (TODO)
            io.write(memory.at(pc+1), a);
            pc++;
            current_cycles += 10;
            //std::cout << "OUT" << std::endl;
            break;
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
            if (flags.cy != 0) {
                pc = memory.at(sp) | (memory.at(sp + 1) << 8);
                sp += 2;
                current_cycles += 11;
                pc--;
            }
            else {
                current_cycles += 5;
            }
            break;
        case 0xD9: // NOP 
            current_cycles += 4;
            break;
        case 0xDA: // JC address 
            if (flags.cy != 0) {
                pc = (memory.at(pc + 2) << 8) | memory.at(pc + 1); // turn next 2 bytes into 16 bit number and store in the PC
                pc--;
            }
            else
                // branch not taken    
                pc += 2;
            //std::cout << "JC address" << std::endl;
            current_cycles += 10;
            break;
        case 0xDB: // IN (TODO)
            a = io.read(memory.at(pc+1));
            pc++;
            current_cycles += 10;
            //std::cout << "IN" << std::endl;
            break;
        case 0xE0: // RPO
            if (flags.p == 0) {
                pc = memory.at(sp) | (memory.at(sp + 1) << 8);
                sp += 2;
                current_cycles += 11;
                pc--;
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
        case 0xE2: // JPO address 
            if (flags.p == 0) { // TODO: Check correctness
                pc = (memory.at(pc + 2) << 8) | memory.at(pc + 1); // turn next 2 bytes into 16 bit number and store in the PC
                pc--;
            }
            else
                // branch not taken    
                pc += 2;
            current_cycles += 10;
            break;
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
            current_cycles += 11; //(TODO)
            break;
        case 0xE6: { // ANI byte
            uint8_t x = a & memory.at(pc + 1);
            logic_flags(x);
            a = x;
            pc++;
            //std::cout << "ANI byte" << std::endl;
            current_cycles += 7;
            break;
        }
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
        case 0xE8: // RPE 
            if (flags.p != 0) {
                pc = memory.at(sp) | (memory.at(sp + 1) << 8);
                sp += 2;
                current_cycles += 11;
                pc--;
            }
            else {
                current_cycles += 5;
            }
            break;
        case 0xE9: { // PCHL
            pc = (h << 8) | l;
            current_cycles += 5;
            break;
        }
        case 0xEC: // CPE address 
            //TODO: Might need to fix
            if (flags.p != 0) {
                uint16_t ret = pc + 3;

                memory.at(sp - 1) = (ret >> 8) & 0xff;
                memory.at(sp - 2) = (ret & 0xff);
                sp -= 2;

                pc = (memory.at(pc + 2) << 8) | memory.at(pc + 1);
                pc--;

                current_cycles += 17;
            }
            else {
                pc += 2;
                current_cycles += 11;
            }
            break;
        case 0xEE: // XRI D8
            a ^= memory.at(pc+1);
            logic_flags(a);
            pc++;
            current_cycles += 7;
            break;
        case 0xf1: { //POP PSW    
            a = memory.at(sp+1);
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
        }
        break;
        case 0xf5: { // PUSH PSW
            memory.at(sp-1) = a;
            uint8_t psw = 0x0;
            if (flags.cy) psw |= 0x01;
            psw |= 0x02;
            if (flags.p) psw |= 0x04;
            if (flags.ac) psw |= 0x10;
            if (flags.z) psw |= 0x40;
            if (flags.s) psw |= 0x80;
            memory.at(sp-2) = psw;
            sp -= 2;
            current_cycles += 11;
            //std::cout <<  "PUSH PSW CALLED" << std::endl;
            //quit = true;
            break;
        }
        case 0xF6: { // ORI D8
            a |= memory.at(pc+1);
            logic_flags(a);
            pc++;
            current_cycles += 7;
            break;
        }
        case 0xFA: // JM address 
            if (flags.s != 0) { // TODO: Check correctness
                pc = (memory.at(pc + 2) << 8) | memory.at(pc + 1); // turn next 2 bytes into 16 bit number and store in the PC
                pc--;
            }
            else
                // branch not taken    
                pc += 2;
            current_cycles += 10;
            break;
        case 0xFB: // EI
            int_enable = true;
            current_cycles += 4;
            //std::cout << "EI" << std::endl;
            break;
        case 0xFC: // CM address 
            if (flags.s != 0) {
                uint16_t ret = pc + 3;

                memory.at(sp - 1) = (ret >> 8) & 0xff;
                memory.at(sp - 2) = (ret & 0xff);
                sp -= 2;
                uint16_t address = (memory.at(pc + 2) << 8) | memory.at(pc + 1);
                pc = address;
                    
                pc--;

                current_cycles += 17;
            }
            else {
                pc += 2;
                current_cycles += 11;
            }
            break;
        case 0xFE: { // CPI byte
            uint8_t x = a - memory.at(pc+1);
            inr_flags(x);
            flags.cy = memory.at(pc + 1) > a;
            pc++;
            current_cycles += 7;
            break;
        }
        case 0xFF:
            memory.at(sp - 1) = (pc >> 8) & 0xFF;
            memory.at(sp - 2) = pc & 0xFF;
            pc = 0x7 * 8;
            sp -= 2;
            current_cycles += 11;
            break;
		default:
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

        // Free the buffer
        delete[] buffer;
    }
}

void Cpu::input() {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                quit = true;
                break;
            default:
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_c:        io.port1 |= 0x01; break;
                    case SDLK_RETURN:   io.port1 |= 0x04; break;
                    case SDLK_SPACE:    io.port1 |= 0x10; break;
                    case SDLK_LEFT:     io.port1 |= 0x20; break;
                    case SDLK_RIGHT:    io.port1 |= 0x40; break;
                }
            case SDL_KEYUP:
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
    if (int_enable) {
        //std::cout << "Interupted!" << std::endl;
        switch (intp) {
            case 0xCF:
                memory.at(sp - 1) = (pc >> 8) & 0xFF;
                memory.at(sp - 2) = pc & 0xFF;
                pc = 0x1 * 8;
                sp -= 2;
                //current_cycles += 11;
                break;
            case 0xD7:
                memory.at(sp - 1) = (pc >> 8) & 0xFF;
                memory.at(sp - 2) = pc & 0xFF;
                pc = 0x2 * 8;
                sp -= 2;
                //current_cycles += 11;
                break;
            default:
                break;
        }
        int_enable = false;
        //f (instructions_read > 1500000) std::cout << std::dec << "Instruction number: " << instructions_read << " opcode: " << std::hex << (intp << 0) << std::endl;
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

void Cpu::add_flags(uint8_t x, uint8_t y, uint8_t carry) {
    uint16_t sum = (uint16_t)x + (uint16_t)y + carry;
    flags.z = ((uint8_t)(x + y + carry) == 0);
    flags.s = ((sum & 0x80) == 0x80);
    flags.p = parity(sum & 0xFF);
    flags.cy = sum > 0xFF;
}

void Cpu::sub_flags(uint8_t x, uint8_t y, uint8_t carry) {
    uint16_t result = (uint16_t)x - (uint16_t)y - carry;
    flags.z = (result == 0);
    flags.s = ((result & 0x80) == 0x80);
    flags.p = parity(uint8_t(x - y - carry));
    flags.ac = ~(x ^ y ^ result) & 0x10;
}

void Cpu::inr_flags(uint8_t x) {
    flags.z = (x == 0);
    flags.s = (x & 0x80) != 0;
    flags.p = parity(x);
}

void Cpu::logic_flags(uint8_t x) {
    flags.z = (x == 0);
    flags.s = ((x & 0x80) != 0);
    flags.p = parity(x);
    flags.cy = 0;
}
