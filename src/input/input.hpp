#pragma once

class input final
{
public:
	static void init();
	static void update();

private:
	static void key_down(SDL_Scancode scancode);
	static void use_item(int num);
	static SDL_Window* window;
};
