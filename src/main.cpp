#include <sdk/SexySDK.hpp>
#include <callbacks/callbacks.hpp>
#include "logger/logger.hpp"
#include "gameplay/gameplay.hpp"
#include "player/player.hpp"
#include "input/input.hpp"
#include "display/display.hpp"
#include "items/items.hpp"
#include "networking/networking.hpp"

void update()
{
	display::update();
	input::update();
}

void cleanup()
{
	networking::disconnect();
}

std::initializer_list<std::string> ext_whitelist
{
	".dll",
	".asi",
};

bool ends_with(std::string const& value, std::string const& ending)
{
	if (ending.size() > value.size()) return false;
	return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

static HANDLE(__stdcall* oCreateFile)(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);

HANDLE __stdcall create_file(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	std::string file_name = std::string(lpFileName);
	std::string path = "data/game/";
	path.append(file_name);

	if (std::filesystem::exists(path))
	{
		return oCreateFile(path.c_str(), dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	}

	return oCreateFile(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

void init()
{
	logger::init("pegroyale");

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		PRINT_ERROR("Failed to start SDL2 (%s)", SDL_GetError());
		return;
	}

	if (enet_initialize() != 0)
	{
		PRINT_ERROR("Failed to start enet");
		return;
	}

	MH_Initialize();

	MH_CreateHookApi(L"kernel32.dll", "CreateFileA", (void**)&create_file, (void**)&oCreateFile);

	networking::init();
	items::init();
	gameplay::init();
	player::init();
	input::init();
	display::init();

	MH_EnableHook(MH_ALL_HOOKS);

	callbacks::on(callbacks::type::main_loop, update);
	std::atexit(cleanup);
}

DWORD WINAPI OnAttachImpl(LPVOID lpParameter)
{
	init();
	return 0;
}

DWORD WINAPI OnAttach(LPVOID lpParameter)
{
	__try
	{
		return OnAttachImpl(lpParameter);
	}
	__except (0)
	{
		FreeLibraryAndExitThread((HMODULE)lpParameter, 0xDECEA5ED);
	}

	return 0;
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		CreateThread(nullptr, 0, OnAttach, hModule, 0, nullptr);
		return true;
	}

	return false;
}
