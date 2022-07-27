#include "display.hpp"
#include "player/player.hpp"
#include "logger/logger.hpp"
#include "networking/networking.hpp"
#include "input/input.hpp"

bool display::can_render_text = false;
bool display::ready = false;
float display::items_timeout;
bool display::show_hud = false;

static int(__fastcall* Sexy__FloatingTextMgr__AddText_)(char*, char*);
int __fastcall Sexy__FloatingTextMgr__AddText(char* this_, char* edx)
{
	auto v3 = *((_DWORD*)this_ + 3);

	if (v3 != 0x0)
	{
		return Sexy__FloatingTextMgr__AddText_(this_, edx);
	}
	else
	{
		return 0;
	}
}

void __declspec(naked) add_standard_text_null_fix()
{
	//Actually doesn't fix anything right now
	__asm
	{
		push ebp;
		mov ebp, esp;
		mov eax, [ecx + 104h];
		mov ecx, [eax + 148h];
		push esi;
		call Sexy__FloatingTextMgr__AddText;
		mov esi, eax;

		test esi, esi;
		jz leave_func;

		push 0x00469EC7;
		retn;

	leave_func:
		push 0x00469F21;
		retn;
	}
}

void display::init()
{
	MH_CreateHook((void*)0x00469D20, Sexy__FloatingTextMgr__AddText, (void**)&Sexy__FloatingTextMgr__AddText_);
	//jump(0x00469EB0, add_standard_text_null_fix);


	callbacks::on(callbacks::type::begin_turn_2, []()
	{
		display::ready = true;

		if (display::ready)
		{
			display::can_render_text = true;
			display::items_timeout = 0.0f;
		}
	});

	callbacks::on(callbacks::type::finish_options_dialog, []()
	{
		if (display::ready) display::can_render_text = true;
	});

	callbacks::on(callbacks::type::do_options_dialog, []()
	{
		if (display::ready) display::can_render_text = false;
	});

	callbacks::on(callbacks::type::do_to_menu, []()
	{
		display::ready = false;
		display::can_render_text = false;
	});

	callbacks::on(callbacks::type::show_level_screen, []()
	{
		display::ready = false;
		display::can_render_text = false;
	});

	callbacks::on_begin_shot([](auto, auto)
	{
		if (display::ready) display::items_timeout = 200.0f;
	});
}

void display::update()
{
	if (display::ready)
	{
		if (display::can_render_text)
		{
			Sexy::FloatingText_* header = (Sexy::FloatingText_*)Sexy::LogicMgr::AddStandardText(
							Sexy::Format("%s", networking::player_list[player::attacking].c_str()),
							500.0f,
							-15.0f,
							14
			);
			header->unk_1 = 1;
			header->float_offset_start = 0.0f;
			header->color = 0xFF2929;

			if (display::items_timeout < 200.0f || display::show_hud)
			{
				Sexy::FloatingText_* header = (Sexy::FloatingText_*)Sexy::LogicMgr::AddStandardText(
							Sexy::Format("Items:"),
							110.0f,
							25.0f,
							14
				);
				header->unk_1 = 1;
				header->float_offset_start = 0.0f;
				header->color = 0x008D51;

				for (auto i = 0; i < 3; ++i)
				{
					Sexy::FloatingText_* item;

					if (player::item_inventory[i].name == "")
					{
						item = (Sexy::FloatingText_*)Sexy::LogicMgr::AddStandardText("---", 110.0f, 25.0f + (22.0f * (i + 1)), 14);
					}
					else
					{
						std::string text = Sexy::Format("%i: %s", (i + 1), player::item_inventory[i].name.c_str());

						if (player::item_inventory[i].count >= 2)
						{
							text = Sexy::Format("%i: %s x%i", (i + 1), player::item_inventory[i].name.c_str(), player::item_inventory[i].count);
						}

						item = (Sexy::FloatingText_*)Sexy::LogicMgr::AddStandardText(text, 110.0f, 25.0f + (22.0f * (i + 1)), 14);
						item->color = player::item_inventory[i].color;
					}

					item->unk_1 = 1;
					item->float_offset_start = 0.0f;
				}

				++display::items_timeout;
			}

			if (display::show_hud)
			{
				Sexy::FloatingText_* header = (Sexy::FloatingText_*)Sexy::LogicMgr::AddStandardText(
								Sexy::Format("Players:"),
								500.0f,
								25.0f,
								14
				);
				header->unk_1 = 1;
				header->float_offset_start = 0.0f;

				for (auto i = 0; i < networking::player_list.size(); ++i)
				{
					std::string player_name = networking::player_list[i];
					std::string attacking_player_name = networking::player_list[player::attacking];

					if (player_name == attacking_player_name && player_name != player::username)
					{
						player_name.insert(0, "* ");
					}

					Sexy::FloatingText_* player = (Sexy::FloatingText_*)Sexy::LogicMgr::AddStandardText(
								Sexy::Format("%s", player_name.c_str()),
								500.0f,
								25.0f + (25.0f * (i + 1)),
								14
					);

					//Self
					if (player_name == player::username)
					{
						player->color = 0x008CFF;
					}
					//Attacking player
					else if (player_name == attacking_player_name && player_name != player::username)
					{
						player->color = 0xFF2929;
					}

					player->unk_1 = 1;
					player->float_offset_start = 0.0f;
				}
			}
		}
	}
}
