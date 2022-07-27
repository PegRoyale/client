#include "player.hpp"
#include "networking/networking.hpp"
#include "logger/logger.hpp"

std::vector<item_t> player::item_inventory = { item_t{}, item_t{}, item_t{} };
int player::multiballs = 0;

std::string player::username;
int player::attacking = 0;
bool player::alive = true;

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
	ini_t* ini;

	//This runs before the mod loader switches the directory back, maybe look into a mod init import function eventually for haggle
	if (!std::filesystem::exists("data/pegroyale.ini"))
	{
		//Check again because the file path changes
		if (!std::filesystem::exists("data/pegroyale.ini"))
		{
			const char* ini_default = ""
				"[settings]\n"
				"username = Player\n"

				"[room]\n"
				"name = NEW_ROOM\n"
				"key = KEY\n";



			ini = ini_create(ini_default, strlen(ini_default));
			ini_save(ini, "data/pegroyale.ini");
		}
	}
	else if (std::filesystem::exists("data/pegroyale.ini"))
	{
		ini = ini_load("data/pegroyale.ini");
	}

	player::username = ini_get(ini, "settings", "username");
	player::validate_name();

	networking::room_name = std::string(ini_get(ini, "room", "name"));
	networking::room_key = std::string(ini_get(ini, "room", "key"));

	if (networking::room_key == "")
	{
		networking::room_key = "_";
	}
}

void player::activate_item(int num)
{
	auto state = Sexy::LogicMgr::GetState();

	//Checks to see if you cannot use it right now
	if (!player::item_inventory[num].during_shot && state == Sexy::LogicMgr::State::Shot)
	{
		return;
	}

	if (player::item_inventory[num].use_sound != 0)
	{
		Sexy::SoundMgr::AddSound(player::item_inventory[num].use_sound);
	}

	//Checks to see if in-game powerup
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
	//Or if its custom
	else
	{
		std::string attacking = networking::player_list[player::attacking];
		networking::send_packet(proto_t::USE_POWEWRUP, logger::va("powerup=%i;attacking=%s;", player::item_inventory[num].powerup, attacking.c_str()));
	}

	switch (player::item_inventory[num].powerup)
	{
	case Sexy::PowerupType::SuperGuide:
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
	player::levels_beat = 0;
}

void player::adjust_ball_count(int count, const std::string& player)
{
	if (count == 0)
	{
		count = -1;
	}

	std::string message;

	if (count >= 1)
	{
		Sexy::LogicMgr::IncNumBalls(count, 0, false);
		Sexy::SoundMgr::AddSound(Sexy::Assets::get(Sexy::Asset::SOUND_AAH));

		if (count == 1)
		{
			message = Sexy::Format("%s gave a ball!", player.c_str());
		}
		else
		{
			message = Sexy::Format("%s gave some balls!", player.c_str());
		}

		Sexy::LogicMgr::AddStandardText(message, 330.0f, 200.0f, 48);
	}
	else if (count < 0)
	{
		Sexy::LogicMgr::IncNumBalls(count, 0, true);
		Sexy::SoundMgr::AddSound(Sexy::Assets::get(Sexy::Asset::SOUND_COINSPIN_NO));

		if (count == -1)
		{
			message = Sexy::Format("%s took a ball!", player.c_str());
		}
		else
		{
			message = Sexy::Format("%s took some balls!", player.c_str());
		}

		Sexy::LogicMgr::AddStandardText(message, 330.0f, 200.0f, 48);
	}
}

void player::handle_enemy_powerup(powerup_t powerup, const std::string& user)
{
	switch (powerup)
	{
	case powerup_t::TAKE_BALL:
		player::adjust_ball_count(-1, user);
		break;
	}
}

void player::validate_name()
{
	//Remove non-ascii
	player::username.erase(std::remove_if(player::username.begin(), player::username.end(), [](auto c)
	{
		return !(c >= 0 && c < 128);
	}), player::username.end());

	//If username is default or less than 3 characters
	if (player::username == "Player" || player::username.size() < 3)
	{
		static std::random_device rd;
		static std::mt19937 mt(rd());
		static std::uniform_int_distribution tag(0, 9999);

		player::username = logger::va("Player-%i", tag(mt));
	}

	//If it contains spaces
	player::username = logger::replace(player::username, " ", "_");

	//If over 12 characters, shorten
	if (player::username.size() > 12)
	{
		player::username = player::username.substr(0, 12);
	}
}
