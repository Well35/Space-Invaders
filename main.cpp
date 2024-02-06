#include <iostream>
#include <fstream>
#include <vector>

#include "src/Cpu.h"

int main() {
    Cpu* cpu = new Cpu();

    int instructions_read = 0;

    while (!cpu->quit) {
        cpu->emulate_cycle();
        instructions_read++;
    }
    

    /*for (int i = 0; i < 37402; i++) {
        cpu->emulate_cycle();
        instructions_read++;
    }*/

    std::cout << std::dec << "Instructions read: " << instructions_read - 1 << std::endl;
    cpu->print_cpu_data();

    delete cpu;
}
