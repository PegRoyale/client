#pragma once

enum powerup_t : int
{
	TAKE_BALL = 14,
};

struct item_t
{
	int powerup;
	int use_sound;
	std::string name;
	std::uint32_t color;
	int count;
	bool during_shot;
};

class items
{
public:
	static void init();
	static std::vector<item_t> available_items;

private:
	static void add(item_t item);
};
