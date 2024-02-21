#include <iostream>
#include <fstream>
#include <vector>

#include "src/Cpu.h"
#include "src/Graphics.h"

int main(int argc, char* args[]) {
    Cpu* cpu = new Cpu();
    Graphics* gfx = new Graphics();

    int start = SDL_GetTicks();
    int end = SDL_GetTicks();
    
    while (!cpu->quit) {
        start = SDL_GetTicks();
        if (start - end > 17) {
            end = start;
            cpu->emulate();
            cpu->interupt(0xCF);
            cpu->emulate();
            cpu->interupt(0xD7);
            gfx->draw(cpu);
        }
        cpu->input();
    }

    std::cout << std::dec << "Instructions read: " << cpu->instructions_read - 1 << std::endl;
    cpu->print_cpu_data();

    delete cpu;
    delete gfx;

    return 0;
}
