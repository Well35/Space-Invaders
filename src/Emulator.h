#pragma once

#include "Cpu.h"
#include "Graphics.h"
#include "MachineIO.h"

class Emulator { // Holds all the different parts to the emulator (cpu, display, IO)
public:
	Emulator();
	~Emulator();
	void run();
private:
	Cpu* cpu;
	Graphics* gfx;
	MachineIO* io;
};