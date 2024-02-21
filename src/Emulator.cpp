#include "Emulator.h"
#include <SDL.h>

Emulator::Emulator() {
	cpu = new Cpu();
	gfx = new Graphics();
	io = new MachineIO();
}

Emulator::~Emulator() {
	delete cpu;
	delete gfx;
	delete io;
}

// main while loop function that handles emulation
void Emulator::run() {
	int start = SDL_GetTicks();
	int end = SDL_GetTicks();

    while (!cpu->quit) {
        start = SDL_GetTicks();
        // Will keep the emulator running at around 60 Hz
        if (start - end > 17) {
            end = start;
            cpu->emulate(); // emulates 16666 cycles
            cpu->interupt(0xCF);
            cpu->emulate(); // emulates 16666 cycles
            cpu->interupt(0xD7); 
            gfx->draw(cpu); // draws full screen at once
        }
        cpu->input();
    }

    //std::cout << std::dec << "Instructions read: " << cpu->instructions_read - 1 << std::endl;
    //cpu->print_cpu_data();
}
