#include "input.hpp"
#include "player/player.hpp"
#include "logger/logger.hpp"

SDL_Window* input::window = 0;
HWND input::hwnd = 0;

void input::init()
{

}

void input::update()
{
	while (!input::window)
	{
		input::hwnd = FindWindowA(0, "Peggle Deluxe 1.01");
		input::window = SDL_CreateWindowFrom(hwnd);
		SetWindowTextA(hwnd, "PegRoyale");
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


POINT input::get_cursor()
{
	POINT p;
	GetCursorPos(&p);
	ScreenToClient(hwnd, &p);
	return p;
}

void input::move_cursor(POINT p)
{
	RECT rect = { 0 };
	GetWindowRect(input::hwnd, &rect);
	SetCursorPos((rect.right - p.x) - 400, (rect.bottom - p.y)- 300);
}

void input::left_click()
{
	INPUT input;
	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	SendInput(1, &input, sizeof(INPUT));

	ZeroMemory(&input, sizeof(INPUT));
	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
	SendInput(1, &input, sizeof(INPUT));
}
