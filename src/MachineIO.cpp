#include "MachineIO.h"

MachineIO::MachineIO() {

}

uint8_t MachineIO::read(uint8_t x) {
	if (x == 0x1) {
		return port1;
	}
	else if (x == 0x3) {
		return shift_result();
	}
	return 0u;
}

void MachineIO::write(uint8_t x, uint8_t y) {
	if (x == 0x2) {
		offset = y & 0x7;
	}
	else if (x == 0x3) {
		port3 = y;
	}
	else if (x == 0x5) {
		port5 = y;
	}
	else if (x == 0x4) {
		shift_value = (shift_value >> 8) | (y << 8);
	}
}

uint8_t MachineIO::shift_result() {
	return ((shift_value >> (8 - offset)) & 0xFF);
}
