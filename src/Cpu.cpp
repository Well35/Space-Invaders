#include "Cpu.h"

#include <fstream>
#include <iostream>

Cpu::Cpu() {
	memory = std::vector<uint8_t>(64000); // 64KB of memory

	pc = memory.at(0); // set program counter to start of memory

    // load all 4 roms to their specified location in memory
    load_rom("invaders.h", 0);
    load_rom("invaders.e", 0x1800);
    load_rom("invaders.f", 0x1000);
    load_rom("invaders.g", 0x0800);
}

// TODO: currently bug with storing values in the wrong places
//          grabbing data from the HL address is grabbing 0 instead of the requested data from ram
void Cpu::emulate_cycle() {
	uint8_t opcode = memory.at(pc); // grab an opcode from pc location
    //std::cout << std::hex << (opcode << 0) << std::endl;

    if (pc + 2 >= memory.size()) {
        quit = true;
        return;
    }

    std::cout << std::hex << "pc: " << (pc << 0) << " opcode: " << (opcode << 0) << " Desc: ";

	// Maybe refactor to something more organzied than a large switch statement
	switch (opcode) {
		case 0x00: // NOP
            std::cout << "NOP" << std::endl;
			break;
		case 0x01: // LXI B,word B <- byte 3, C <- byte 2
			c = memory.at(pc + 1);
			b = memory.at(pc + 2);
            std::cout << "LXI B,word" << std::endl;
            pc += 2;
			break;
        case 0x05: // DEC B
            b--;
            flags.z = (b == 0);
            flags.s = (b & (1 << 7));
            //TODO: parity
            //TODO: ac flag
            std::cout << "DEC B " << flags.s << std::endl;
            break;
        case 0x06: // MVI B, D8
            b = memory.at(pc + 1);
            pc++;
            std::cout << "MVI B, D8" << std::endl;    
            break;
        case 0x09: { // DAD B
            uint32_t x = (h << 8) | l;
            uint32_t y = (b << 8) | c;
            x += y;
            flags.cy = (x > 0xffff);
            l = x & 0xFF;
            h = x >> 8;
            std::cout << std::hex << "DAD D x: " << (x << 0) << " h: " << (h << 0) << " l: " << (l << 0) << std::endl;
            break;
        }
        case 0x0D: // DEC C
            c--;
            flags.z = (c == 0);
            flags.s = (c & (1 << 7));
            //TODO: parity
            //TODO: ac flag
            std::cout << "DEC C " << flags.s << std::endl;
            break;
        case 0x0E: // MVI C,D8
            c = memory.at(pc + 1);
            pc++;
            std::cout << "MVI C,D8" << std::endl;
            break;
        case 0x0F: { //RRC
            uint8_t x = a;
            a = ((x & 1) << 7) | (x >> 1);
            flags.cy = (1 == (x & 1));
            std::cout << "RRC" << std::endl;
            break;
        }
        case 0x11: // LXI D,D16
            e = memory.at(pc + 1);
            d = memory.at(pc + 2);
            pc += 2;
            std::cout << "LXI D,D16" << std::endl;
            break;
        case 0x13: { // INX D
            uint16_t x = (d << 8) | e;
            x += 1;
            e = x & 0xFF;
            d = x >> 8;
            std::cout << std::hex << "INX D x: " << (x << 0) << " d: " << (h << 0) << " e: " << (l << 0) << std::endl;
            break;
        }
        case 0x19: { // DAD D
            uint32_t x = (h << 8) | l;
            uint32_t y = (d << 8) | e;
            x += y;
            flags.cy = (x > 0xffff);
            l = x & 0xFF;
            h = x >> 8;
            std::cout << std::hex << "DAD D x: " << (x << 0) << " h: " << (h << 0) << " l: " << (l << 0) << std::endl;
            break;
        }
        case 0x1A: // LDAXD
            // not sure if this is right
            a = memory.at(d << 8 | e);
            std::cout << "LDAXD " << std::hex << (a << 0) << std::endl;
            break;
        case 0x1f: { // RRA
            uint8_t x = a;
            a = (flags.cy << 7) | (x >> 1);
            flags.cy = (1 == (x & 1));
            break;
        }
        case 0x21: // LXI H,D16
            l = memory.at(pc + 1);
            h = memory.at(pc + 2);
            pc += 2;
            std::cout << "LXI H,D16" << std::endl;
            break;
        case 0x23: { // INX H
            uint16_t x = (h << 8) | l;
            x += 1;
            l = x & 0xFF;
            h = x >> 8;
            std::cout << std::hex << "INX H x: " << (x << 0) << " h: " << (h << 0) << " l: " << (l << 0) << std::endl;
            break;
        }
        case 0x26: // MVI H,D8
            h = memory.at(pc + 1);
            pc++;
            std::cout << "MVI H,D8" << std::endl;
            break;
        case 0x29: { // DAD H
            uint32_t x = (h << 8) | l;
            x += x;
            flags.cy = (x > 0xffff);
            l = x & 0xFF;
            h = x >> 8;
            std::cout << std::hex << "DAD H x: " << (x << 0) << " h: " << (h << 0) << " l: " << (l << 0) << std::endl;
            break;
        }
        /*....*/
        case 0x2F: // CMA
            a = ~a;
            std::cout << "CMA" << std::endl;
            break;
        case 0x31: // LXI SP,D16
            sp = (memory.at(pc + 2) << 8) | memory.at(pc + 1);
            pc += 2;
            std::cout << "LXI SP,D16" << std::endl;
            break;
        case 0x32: // STA adr
            memory.at(memory.at(pc + 1) << 8 | memory.at(pc + 2)) = a;
            pc += 2;
            std::cout << "STA adr" << std::endl;
            break;
        case 0x36: // MVI M,D8
            memory.at(h << 8 | l) = memory.at(pc+1);
            pc++;
            std::cout << "MVI M,D8 " << std::endl;
            break;
        case 0x3A: // LDA adr
            a = memory.at(memory.at(pc+1) << 8 | memory.at(pc+2));
            pc += 2;
            std::cout << "LDA adr" << std::endl;
            break;
        case 0x3E: // MVI A,D8
            a = memory.at(pc + 1);
            pc++;
            std::cout << "MVI A,D8" << std::endl;
            break;
        case 0x41: // MOV B,C
            b = c;
            std::cout << "MOV B,C" << std::endl;
            break;
        case 0x42: // MOV B,D
            b = d;
            std::cout << "MOV B,D" << std::endl;
            break;
        case 0x43: // MOV B,E
            b = e;
            std::cout << "MOV B,E" << std::endl;
            break;
        case 0x56: // MOVE D,M
            d = memory.at(h << 8 | l);
            std::cout << "MOV D,M" << std::endl;
            break;
        case 0x5E: // MOVE E,M
            e = memory.at(h << 8 | l);
            std::cout << "MOV E,M" << std::endl;
            break;
        case 0x66: // MOVE H,M
            h = memory.at(h << 8 | l);
            std::cout << "MOV H,M" << std::endl;
            break;
        case 0x6F: // MOV L,A
            l = a;
            std::cout << "MOV L,A" << std::endl;
            break;
        case 0x77: // MOV M,A
            // TODO: check correctness
            memory.at(h << 8 | l) = c;
            std::cout << "MOV M,A " << std::hex << (memory.at(h << 8 | l) << 0) << std::endl;
            break;
        case 0x7A: // MOV A,D
            a = d;
            std::cout << "MOV A,D" << std::endl;
            break;
        case 0x7B: // MOV A,E
            a = e;
            std::cout << "MOV A,E" << std::endl;
            break;
        case 0x7C: // MOV A,H
            a = h;
            std::cout << "MOV A,H" <<  std::endl;
            break;
        /*....*/
        case 0x7E: // MOVE A,M
            //TODO: might have to fix this
            // storing to that memory location might have a bug
            a = memory.at(h << 8 | l);
            std::cout << "MOV A,M" << std::endl;
            break;
        case 0x80: { // ADD B
            uint16_t answer = (uint16_t)a + (uint16_t)b;
            flags.z = ((answer & 0xff) == 0);
            flags.s = (answer & (1 << 7)); // if bit 7 is set, set signed flag to 1
            flags.cy = (answer > 0xff); // set carry flag if greater than max
            //TODO: add parity
            a = answer & 0xff; // back to 8 bit
            std::cout << "ADD B" << std::endl;
            break;
        }
        case 0x81: { // ADD C
            uint16_t answer = (uint16_t)a + (uint16_t)c;
            flags.z = ((answer & 0xff) == 0);
            flags.s = (answer & (1 << 7)); // if bit 7 is set, set signed flag to 1
            flags.cy = (answer > 0xff); // set carry flag if greater than max
            //TODO: add parity
            a = answer & 0xff; // back to 8 bit
            std::cout << "ADD C" << std::endl;
            break;
        }
        /*....*/
        case 0xA7: // ANA A
            a &= a;
            flags.z = ((a & 0xff) == 0);
            flags.s = (a & (1 << 7));
            flags.cy = (a > 0xff);
            //TODO: add parity
            //TODO: ac flag
            std::cout << "ANA A" << std::endl;
            break;
        case 0xAF: // XRA A
            a ^= a;
            flags.z = ((a & 0xff) == 0);
            flags.s = (a & (1 << 7));
            flags.cy = (a > 0xff);
            //TODO: add parity
            //TODO: ac flag
            std::cout << "XRA A" << std::endl;
            break;
        case 0xC1: { // POP B
            c = memory.at(sp);
            b = memory.at(sp+1);
            sp += 2;
            std::cout << "POP B" << std::endl;
            break;
        }
        case 0xC2: // JNZ address 
            if (flags.z == 0) {
                pc = (memory.at(pc + 2) << 8) | memory.at(pc + 1); // turn next 2 bytes into 16 bit number and store in the PC
                pc--;
            }
            else
                // branch not taken    
                pc += 2;
            std::cout << "JNZ address" << std::endl;
            break;
        case 0xC3: // JMP address
            pc = (memory.at(pc + 2) << 8) | memory.at(pc + 1);
            pc--;
            std::cout << "JMP address " << std::hex << (pc + 1 << 0) << std::endl;
            break;
        case 0xC5: { // PUSH B
            memory.at(sp-1) = b;
            memory.at(sp-2) = c;
            sp -= 2;
            std::cout << "PUSH B" << std::endl;
            break;
        }
        case 0xC6: { // ADI byte
            uint16_t answer = (uint16_t)a + (uint16_t)memory.at(pc + 1);
            flags.z = ((answer & 0xff) == 0);
            flags.s = (answer & (1 << 7)); // if bit 7 is set, set signed flag to 1
            flags.cy = (answer > 0xff); // set carry flag if greater than max
            //TODO: add parity
            a = answer & 0xff; // back to 8 bit
            pc++;
            std::cout << "ADI byte" << std::endl;
            break;
        }
        case 0xCD: { // CALL address
            uint16_t ret = pc + 2;

            memory.at(sp - 1) = (ret >> 8) & 0xff; // store the return location into 2 bytes of memory
            memory.at(sp - 2) = (ret & 0xff);
            sp -= 2;

            pc = (memory.at(pc + 2) << 8) | memory.at(pc + 1); // move pc to the call address
            pc--;  // revert the pc++

            std::cout << "CALL address" << std::endl;
            break;
        }
        case 0xC9: // RET address
            pc = memory.at(sp) | (memory.at(sp + 1) << 8);
            sp += 2;
            std::cout << "RET address" << std::endl;
            break;
        case 0xD1: { // POP D
            e = memory.at(sp);
            d = memory.at(sp + 1);
            sp += 2;
            std::cout << "POP D" << std::endl;
            break;
        }
        case 0xD3: // OUT (TODO)
            pc++;
            std::cout << "OUT" << std::endl;
            break;
        case 0xD5: // PUSH D
            memory.at(sp - 2) = e;
            memory.at(sp - 1) = d;
            sp -= 2;
            std::cout << "PUSH D" << std::endl;
            break;
        case 0xE1: { // POP H
            l = memory.at(sp);
            h = memory.at(sp + 1);
            sp += 2;
            std::cout << "POP H" << std::endl;
            break;
        }
        case 0xE5: // PUSH H
            memory.at(sp - 2) = l;
            memory.at(sp - 1) = h;
            sp -= 2;
            std::cout << "PUSH H" << std::endl;
            break;
        case 0xE6: { // ANI byte
            uint8_t x = a & memory.at(pc + 1);
            flags.z = (x == 0);
            flags.s = (0x80 == (x & 0x80));
            // parity
            flags.cy = 0;
            a = x;
            pc++;
            std::cout << "ANI byte" << std::endl;
            break;
        }
        case 0xEB: { // XCHG
            uint8_t x = h;
            h = d;
            d = x;

            x = l;
            l = e;
            e = x;
            std::cout << "XCHG" << std::endl;
            break;
        }
        case 0xf1: { //POP PSW    
            a = memory.at(sp+1);
            uint8_t psw = memory.at(sp);
            flags.z = (0x01 == (psw & 0x01));
            flags.s = (0x02 == (psw & 0x02));
            flags.p = (0x04 == (psw & 0x04));
            flags.cy = (0x05 == (psw & 0x08));
            flags.ac = (0x10 == (psw & 0x10));
            sp += 2;
        }
        break;
        case 0xf5: { // PUSH PSW
            memory.at(sp-1) = a;
            uint8_t psw = (uint8_t(flags.z) |
                           uint8_t(flags.s) << 1 |
                           uint8_t(flags.p) << 2 |
                           uint8_t(flags.cy) << 3 |
                           uint8_t(flags.ac) << 4);
            memory.at(sp-2) = psw;
            sp -= 2;
            break;
        }
        case 0xFB: // EI
            int_enable = true;
            std::cout << "EI" << std::endl;
            break;
        case 0xFE: { // CPI byte
            uint8_t x = a - memory.at(pc+1);
            flags.z = (x == 0);
            flags.s = (0x80 == (x & 0x80));
            //TODO: parity
            flags.cy = (a < memory.at(pc+1));
            pc++;
            break;
        }
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

        std::cout << std::hex << "Loading rom: " << rom_name << " from " << (start_pos << 0) << " to " << ((start_pos + size) << 0) << std::endl;
        for (uint32_t i = 0; i < size; i++) {
            //std::cout << std::hex << (memory.at(i) << 0) << std::endl;
            memory.at(i+start_pos) = buffer[i];
        }

        // Free the buffer
        delete[] buffer;
    }
}

void Cpu::print_cpu_data() {
    std::cout << std::hex << " a: " << (a << 0) << " b: " << (b << 0) << " c: " << (c << 0) << " d: " << (d << 0) << " e: " << (e << 0) << " h: " << (h << 0) << " l: " << (l << 0) << std::endl;
    std::cout << "flags: " << flags.z << flags.s << flags.p << flags.cy << flags.ac << std::endl;
    std::cout << "sp: " << std::hex << (sp << 0) << " pc: " << (pc << 0) << std::endl;
}
