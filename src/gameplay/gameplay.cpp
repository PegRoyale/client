#include "gameplay.hpp"
#include "display/display.hpp"
#include "player/player.hpp"
#include "items/items.hpp"
#include "logger/logger.hpp"
#include "networking/networking.hpp"
#include "input/input.hpp"

int gameplay::balls_left = 10;
Sexy::PhysObj* gameplay::last_phys_obj;
Sexy::Ball* gameplay::last_ball;
bool gameplay::prompted_leave = true;

void __declspec(naked) next_boards_balls_hook()
{
	static constexpr std::uint32_t retn_addr = 0x0045DE61;
	__asm
	{
		mov eax, gameplay::balls_left;
		push retn_addr;
		retn;
	}
}

void gameplay::purple_peg_hit()
{
	Sexy::PhysObj_* phys_obj_ = (Sexy::PhysObj_*)gameplay::last_phys_obj;
	double pos_x = ((double(__thiscall*)(Sexy::PhysObj*)) * (_DWORD*)(*(_DWORD*)phys_obj_->data + 120))(gameplay::last_phys_obj);
	double pos_y = ((double(__thiscall*)(Sexy::PhysObj*)) * (_DWORD*)(*(_DWORD*)phys_obj_->data + 124))(gameplay::last_phys_obj);

	Sexy::FloatingText_* flt_text = (Sexy::FloatingText_*)Sexy::LogicMgr::AddStandardText("Readying Attack!", (float)pos_x, (float)pos_y, -1);
	flt_text->color = 0xFF88FF;
	flt_text->unk_2 = -35;
	flt_text->float_offset_start = -0.5f;
	flt_text->unk_5 = 10;
	flt_text->unk_1 = 150;
}

void gameplay::green_peg_hit()
{
	static std::random_device rd;
	static std::mt19937 mt(rd());
	static int count = items::available_items.size() - 1;
	static std::uniform_int_distribution rand(0, count);

	for (auto i = 0; i < 3; ++i)
	{
		if (player::item_inventory[i].name == "")
		{
			item_t new_item;
			new_item = items::available_items[rand(mt)];
			player::item_inventory[i] = new_item;

			Sexy::PhysObj_* phys_obj_ = (Sexy::PhysObj_*)gameplay::last_phys_obj;
			double pos_x = ((double(__thiscall*)(Sexy::PhysObj*)) * (_DWORD*)(*(_DWORD*)phys_obj_->data + 120))(gameplay::last_phys_obj);
			double pos_y = ((double(__thiscall*)(Sexy::PhysObj*)) * (_DWORD*)(*(_DWORD*)phys_obj_->data + 124))(gameplay::last_phys_obj);

			Sexy::FloatingText_* flt_text = (Sexy::FloatingText_*)Sexy::LogicMgr::AddStandardText(new_item.name.append(" Got!"), (float)pos_x, (float)pos_y, -1);
			flt_text->color = 0x88FF88;
			flt_text->unk_2 = -35;
			flt_text->float_offset_start = -0.5f;
			flt_text->unk_5 = 10;
			flt_text->unk_1 = 150;

			break;
		}
	}
}

void __declspec(naked) purple_peg_hit_hook()
{
	__asm
	{
		cmp [ecx + 10h], ebx;
		jnz loc_46FE4D;
		call gameplay::purple_peg_hit;

	loc_46FE4D:
		push 0x0046FE4D;
		retn;
	}
}

void __declspec(naked) green_peg_hit_hook()
{
	__asm
	{
		cmp dword ptr[edx + 10h], 4;
		jnz loc_4701F0;
		call gameplay::green_peg_hit;

	loc_4701F0:
		push 0x004701F0;
		retn;
	}
}

static int(__fastcall* Sexy__Board__Pause_)(Sexy::Board*, char*, bool);
int __fastcall Sexy__Board__Pause(Sexy::Board* this_, char* edx, bool pause)
{
	return 0;
	//auto retn = Sexy__Board__Pause_(this_, edx, pause);
}

static int(__fastcall* Sexy__ThunderballApp__GetMaxLockedStage_)(Sexy::ThunderballApp*, char*);
int __fastcall Sexy__ThunderballApp__GetMaxLockedStage(Sexy::ThunderballApp* this_, char* edx)
{
	return 1000;
}

static int(__fastcall* Sexy__ThunderballApp__GetMaxLockedLevel_)(Sexy::ThunderballApp*, char*);
int __fastcall Sexy__ThunderballApp__GetMaxLockedLevel(Sexy::ThunderballApp* this_, char* edx)
{
	return 1000;
}

static char*(__fastcall* Sexy__EndLevelDialog__EndLevelDialog_)(char*, char*, int, bool);
char* __fastcall Sexy__EndLevelDialog__EndLevelDialog(char* this_, char* edx, int a2, bool a3)
{
	auto retn = Sexy__EndLevelDialog__EndLevelDialog_(this_, edx, a2, a3);

	gameplay::prompted_leave = true;
	display::can_render_text = false;
	++player::levels_beat;

	return retn;
}

void gameplay::init()
{
	jump(0x0045DE5C, next_boards_balls_hook); //Stores balls left over to roll over into the next board
	jump(0x0046FD82, purple_peg_hit_hook); //For attacks
	jump(0x004701C8, green_peg_hit_hook); //For powerups
	//jump(0x004A9286, 0x004A9291); //Instantly load level

	MH_CreateHook((void*)0x004025C0, Sexy__Board__Pause, (void**)&Sexy__Board__Pause_);
	MH_CreateHook((void*)0x00405CF0, Sexy__ThunderballApp__GetMaxLockedStage, (void**)&Sexy__ThunderballApp__GetMaxLockedStage_);
	MH_CreateHook((void*)0x00405D40, Sexy__ThunderballApp__GetMaxLockedLevel, (void**)&Sexy__ThunderballApp__GetMaxLockedLevel_);
	MH_CreateHook((void*)0x0040AC40, Sexy__EndLevelDialog__EndLevelDialog, (void**)&Sexy__EndLevelDialog__EndLevelDialog_);


	callbacks::after_begin_turn_2([](auto logic_mgr)
	{
		Sexy::LogicMgr_* logic_mgr_ = (Sexy::LogicMgr_*)logic_mgr;

		std::uint32_t ptr_ = *(std::uint32_t*)&logic_mgr_->data[284];
		int balls_left = *(std::uint32_t*)&logic_mgr_->data[4 * ptr_ + 368];
		gameplay::balls_left = balls_left;
	});

	callbacks::on_peg_hit([](auto ball, auto phys_obj, auto)
	{
		gameplay::last_phys_obj = phys_obj;
		gameplay::last_ball = ball;

		if (player::multiballs)
		{
			Sexy::LogicMgr::DoPowerup(ball, phys_obj, Sexy::PowerupType::Multiball, 0);
			--player::multiballs;
		}
	});

	//We lost, return to menu
	//Possible show a game over message, what rank they made it to, stats, etc
	callbacks::on(callbacks::type::after_beat_level_false, []()
	{
		player::alive = false;
		gameplay::reset();
		networking::send_packet(proto_t::DIED);
		MessageBoxA(nullptr, "Game Over!", "PegRoyale", 0);
		exit(0);
	});

	callbacks::on(callbacks::type::do_to_menu, []()
	{
		if (!gameplay::prompted_leave)
		{
			exit(0);
		}
		else if(gameplay::prompted_leave)
		{
			//gameplay::prompted_leave = false;
			Sexy::ThunderballApp::ShowLevelScreen(true);
		}
	});

	callbacks::on(callbacks::type::after_show_level_screen, []()
	{
		if (!gameplay::prompted_leave)
		{
			exit(0);
		}
		else if (gameplay::prompted_leave)
		{
			if (!networking::ready_up)
			{
				networking::ready_up = true;
				networking::send_packet(proto_t::READY_UP);

				while (networking::wait_for_others)
				{
					std::this_thread::sleep_for(150ms);
				}
			}

			gameplay::prompted_leave = false;
			Sexy::LevelScreen::DoPlay(networking::level_order[player::levels_beat]);
		}
	});
}

void gameplay::reset()
{
	gameplay::balls_left = 10;
	display::can_render_text = false;
	player::reset();
}
