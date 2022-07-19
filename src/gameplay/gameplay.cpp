#include "gameplay.hpp"
#include "logger/logger.hpp"

int gameplay::balls_left = 10;

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
	PRINT_DEBUG("Ready attack...");
}

void gameplay::green_peg_hit()
{
	PRINT_DEBUG("Give powerup");
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

	callbacks::after_begin_turn_2([](auto logic_mgr)
	{
		Sexy::LogicMgr_* logic_mgr_ = (Sexy::LogicMgr_*)logic_mgr;

		std::uint32_t ptr_ = *(std::uint32_t*)&logic_mgr_->data[284];
		int balls_left = *(std::uint32_t*)&logic_mgr_->data[4 * ptr_ + 368];
		gameplay::balls_left = balls_left;
	});

	//We lost, return to menu
	//Possible show a game over message, what rank they made it to, stats, etc
	callbacks::on(callbacks::type::after_beat_level_false, []()
	{
		Sexy::ThunderballApp::DoToMenu();
	});

	//Completed a level, back out to menu to load the next level as this is a requirement
	callbacks::on(callbacks::type::after_beat_level_true, []()
	{
		Sexy::ThunderballApp::DoToMenu();
		Sexy::ThunderballApp::ShowLevelScreen(false);
		Sexy::LevelScreen::DoPlay(-1);
	});
}
