#include "input.hpp"
#include "player/player.hpp"
#include "logger/logger.hpp"

SDL_Window* input::window = 0;

void input::init()
{

}

void input::update()
{
	while (!input::window)
	{
		HWND hwnd = FindWindowA(0, "Peggle Deluxe 1.01");
		input::window = SDL_CreateWindowFrom(hwnd);
	}

	static SDL_Event evt;

	while (SDL_PollEvent(&evt))
	{
		switch (evt.type)
		{
		case SDL_KEYDOWN:
			input::key_down(evt.key.keysym.scancode);
			break;
		}
	}
}

void input::key_down(SDL_Scancode scancode)
{
	switch (scancode)
	{
	case SDL_SCANCODE_1:
		input::use_item(0);
		break;

	case SDL_SCANCODE_2:
		input::use_item(1);
		break;

	case SDL_SCANCODE_3:
		input::use_item(2);
		break;
	}
}

void input::use_item(int num)
{
	player::activate_item(num);
}
