#include "input.hpp"
#include "player/player.hpp"
#include "display/display.hpp"
#include "logger/logger.hpp"
#include "networking/networking.hpp"

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
		SetWindowTextA(hwnd, logger::va("PegRoyale | Room: %s | Players: %i", networking::room_name.c_str(), 1).c_str());
	}

	static SDL_Event evt;

	while (SDL_PollEvent(&evt))
	{
		switch (evt.type)
		{
		case SDL_KEYDOWN:
			input::key_down(evt.key.keysym.scancode);
			break;

		case SDL_KEYUP:
			input::key_up(evt.key.keysym.scancode);
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

	case SDL_SCANCODE_TAB:
		display::show_players = true;
		break;

	case SDL_SCANCODE_Q:
		if (networking::player_list.size() == 1) return;

		player::attacking--;

		if (player::attacking < 0)
		{
			player::attacking = networking::player_list.size() - 1;
		}

		if (networking::player_list[player::attacking] == player::username)
		{
			player::attacking--;
		}

		if (player::attacking < 0)
		{
			player::attacking = networking::player_list.size() - 1;
		}
		break;

	case SDL_SCANCODE_E:
		if (networking::player_list.size() == 1) return;

		player::attacking++;

		if (player::attacking > networking::player_list.size() - 1)
		{
			player::attacking = 0;
		}

		if (networking::player_list[player::attacking] == player::username)
		{
			player::attacking++;
		}

		if (player::attacking > networking::player_list.size() - 1)
		{
			player::attacking = 0;
		}
		break;
	}
}

void input::key_up(SDL_Scancode scancode)
{
	switch (scancode)
	{
	case SDL_SCANCODE_TAB:
		display::show_players = false;
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
