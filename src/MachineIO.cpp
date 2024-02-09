#include "MachineIO.h"

#include <iostream>

uint8_t MachineIO::read(uint8_t x) {
	if (x == 0) {
		return 1;
	}
	if (x == 1) {
		return port1;
	}
	if (x == 3) {
		return shift_result();
	}
	return 0;
}

void MachineIO::write(uint8_t x, uint8_t y) {
	if (x == 2) {
		offset = y & 0x7;
	}
	else if (x == 4) {
		shift0 = shift1;
		shift1 = y;
	}
}

uint8_t MachineIO::shift_result() {
	uint16_t v = (shift1 << 8) | shift0;
	return ((v >> (8 - offset)) & 0xff);
	//return ((shift_value >> (8 - offset)) & 0xFF);
}
