#pragma once

#include <SDL.h>
#include "Cpu.h"

class Graphics {
public:
	Graphics();

	void draw(const Cpu* cpu);
private:
	int window_width = 256;
	int window_height = 224;
	int scale = 3;

	SDL_Window* window;
	SDL_Renderer* renderer;
};