#pragma once

struct item_t
{
	int powerup;
	int use_sound;
	std::string name;
	std::uint32_t color;
	int count;
	bool on_next_turn;
};

class items
{
public:
	static void init();
	static std::vector<item_t> available_items;

private:
	static void add(item_t item);
};
