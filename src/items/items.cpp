#include "items.hpp"

std::vector<item_t> items::available_items;

void items::init()
{
	item_t super_guide;
	super_guide.powerup = Sexy::PowerupType::SuperGuide;
	super_guide.name = std::string("Super Guide");
	super_guide.color = 0x436BAB;
	super_guide.count = 3;
	super_guide.on_next_turn = true;
	super_guide.use_sound = Sexy::Assets::get(Sexy::Asset::SOUND_POWERUP_GUIDE);
	items::add(super_guide);

	item_t multiball;
	multiball.powerup = Sexy::PowerupType::Multiball;
	multiball.name = std::string("Multiball");
	multiball.color = 0xED920E;
	multiball.count = 1;
	multiball.on_next_turn = false;
	multiball.use_sound = 0; //FIXME
	items::add(multiball);
}

void items::add(item_t item)
{
	items::available_items.emplace_back(item);
}
