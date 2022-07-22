#include "items.hpp"

std::vector<item_t> items::available_items;

void items::init()
{
	item_t super_guide;
	super_guide.powerup = Sexy::PowerupType::SuperGuide;
	super_guide.name = std::string("Super Guide");
	super_guide.color = 0x436BAB;
	super_guide.count = 3;
	super_guide.during_shot = false;
	super_guide.use_sound = Sexy::Assets::get(Sexy::Asset::SOUND_POWERUP_GUIDE);
	items::add(super_guide);

	item_t multiball;
	multiball.powerup = Sexy::PowerupType::Multiball;
	multiball.name = std::string("Multiball");
	multiball.color = 0xED920E;
	multiball.count = 1;
	multiball.during_shot = true;
	multiball.use_sound = Sexy::Assets::get(Sexy::Asset::SOUND_AAH); //FIXME
	items::add(multiball);

	item_t take_ball;
	take_ball.powerup = powerup_t::TAKE_BALL;
	take_ball.name = std::string("Take Ball");
	take_ball.color = 0xE33D3D;
	take_ball.count = 1;
	take_ball.during_shot = true;
	take_ball.use_sound = Sexy::Assets::get(Sexy::Asset::SOUND_COINSPIN_NO); //FIXME
	items::add(take_ball);
}

void items::add(item_t item)
{
	items::available_items.emplace_back(item);
}
