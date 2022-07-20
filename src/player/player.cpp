#include "player.hpp"

std::vector<item_t> player::item_inventory = { item_t{}, item_t{}, item_t{} };
int player::multiballs = 0;

//General statistics to show the user
int player::levels_beat;
int player::shots_fired;
int player::pegs_hist;
int player::orange_pegs_hit;
int player::purple_pegs_hit;
int player::green_pegs_hit;
int player::powerups_used;
int player::buckets_sunk;

void player::init()
{

}

void player::activate_item(int num)
{
	bool single = true;

	Sexy::SoundMgr::AddSound(player::item_inventory[num].use_sound);

	if (player::item_inventory[num].powerup >= 1 && player::item_inventory[num].powerup <= 13)
	{
		switch (player::item_inventory[num].powerup)
		{
		case Sexy::PowerupType::Multiball:
			++player::multiballs;
			break;

		case Sexy::PowerupType::Pyramid:
			break;

		case Sexy::PowerupType::LuckySpin:
			break;

		default:
			Sexy::LogicMgr::ActivatePowerup(player::item_inventory[num].powerup, 1);
			break;
		}
	}
	else
	{
		//Custom
	}

	switch (player::item_inventory[num].powerup)
	{
	case Sexy::PowerupType::SuperGuide:
		single = false;
		Sexy::LogicMgr::ActivatePowerup(Sexy::PowerupType::SuperGuide, 1);
		break;
	}

	--player::item_inventory[num].count;

	if (player::item_inventory[num].count <= 0)
	{
		player::item_inventory[num] = item_t{};
	}
}

void player::reset()
{
	player::multiballs = 0;
}
