#pragma once

class input final
{
public:
	static void init();
	static void update();
	static void move_cursor(POINT p);
	static POINT get_cursor();
	static void left_click();
	static HWND hwnd;

private:
	static void key_down(SDL_Scancode scancode);
	static void key_up(SDL_Scancode scancode);
	static void use_item(int num);
	static SDL_Window* window;
};
