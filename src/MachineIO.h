#pragma once

#include <SDL.h>

class MachineIO {
public:
	uint8_t read(uint8_t x);
	void write(uint8_t x, uint8_t y);

	uint8_t port1 = 0;
	uint8_t port3 = 0;
	uint8_t port5 = 0;

	uint8_t shift0 = 0;
	uint8_t shift1 = 0;
private:
	

	uint8_t offset = 0;

	uint8_t shift_result();
};