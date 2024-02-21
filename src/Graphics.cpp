#include "Graphics.h"

#include <iostream>

Graphics::Graphics() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("Could not initialize SDL subsystems! %s\n", SDL_GetError());
    }

    window = SDL_CreateWindow("Space Invaders", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_height * scale, window_width * scale, 0);
    if (!window) {
        SDL_Log("Could not create SDL window %s\n", SDL_GetError());
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_Log("Could not create SDL renderer %s\n", SDL_GetError());
    }
}

void Graphics::draw(const Cpu* cpu) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_Rect r = {0, 0, scale, scale};

    uint16_t vram_start_location = 0x2400;

    //std::cout << "Drawing!" << std::endl;

    for (uint16_t i = 0; i < window_height; i++) {
        for (uint16_t j = 0; j < window_width; j++) {
            uint16_t v = 0x20 * i;
            uint16_t byte_to_draw = vram_start_location + v + (j >> 3);
            uint16_t bit_to_check = j % 8;

            bool pixel = (cpu->memory.at(byte_to_draw) & (1 << bit_to_check)) != 0;

            if (pixel) SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            else SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);

            r.x = i * scale;
            r.y = (256-j-1) * scale; //(256-i-1) * scale;

            SDL_RenderFillRect(renderer, &r);
        }
    }


    SDL_RenderPresent(renderer);
}
