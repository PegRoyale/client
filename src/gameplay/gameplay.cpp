#include "gameplay.hpp"
#include "display/display.hpp"
#include "player/player.hpp"
#include "items/items.hpp"
#include "logger/logger.hpp"

int gameplay::balls_left = 10;
Sexy::PhysObj* gameplay::last_phys_obj;
Sexy::Ball* gameplay::last_ball;

void __declspec(naked) next_board_balls_hook()
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

void gameplay::init()
{
	jump(0x0045DE5C, next_board_balls_hook); //Stores balls left over to roll over into the next board
	jump(0x0046FD82, purple_peg_hit_hook); //For attacks
	jump(0x004701C8, green_peg_hit_hook); //For powerups
	jump(0x004A9286, 0x004A9291); //Instantly load level

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
		gameplay::reset();
		Sexy::ThunderballApp::DoToMenu();
	});

	//We completed a level, back out to menu to load the next level as this is a requirement
	callbacks::on(callbacks::type::after_beat_level_true, []()
	{
		display::can_render_text = false;
		Sexy::ThunderballApp::DoToMenu();
		Sexy::ThunderballApp::ShowLevelScreen(true);
	});
}

void gameplay::reset()
{
	gameplay::balls_left = 10;
	display::can_render_text = false;
	player::reset();
}
