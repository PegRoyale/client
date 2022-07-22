#pragma once

#include "items/items.hpp"

class player final
{
public:
	static void init();
	static void activate_item(int num);
	static void reset();
	static void handle_enemy_powerup(powerup_t powerup, const std::string& user = "A Player");
	static void adjust_ball_count(int count, const std::string& player = "A Player");

	static std::vector<item_t> item_inventory;

	static std::string username;
	static int attacking;
	static bool alive;

	//For determining powerups that require a peg hit
	static int multiballs;

	//General statistics to show the user
	static int levels_beat;
	static int shots_fired;
	static int pegs_hist;
	static int orange_pegs_hit;
	static int purple_pegs_hit;
	static int green_pegs_hit;
	static int powerups_used;
	static int buckets_sunk;

private:
	static void push_stats();
	static void pull_stats();
	static void validate_name();
};
