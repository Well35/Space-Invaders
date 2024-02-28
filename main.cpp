#include <iostream>
#include <fstream>
#include <vector>

#include "src/Emulator.h"

int main(int argc, char* args[]) {

    Emulator* space_invaders_emulator = new Emulator();
    space_invaders_emulator->run();

    return 0;
}
