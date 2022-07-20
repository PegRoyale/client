#include "display.hpp"
#include "player/player.hpp"
#include "logger/logger.hpp"

bool display::can_render_text = false;
bool display::ready = false;
float display::items_timeout;

void display::init()
{
	callbacks::on(callbacks::type::begin_turn_2, []()
	{
		display::ready = true;

		if (display::ready)
		{
			display::can_render_text = true;
			display::items_timeout = 0.0f;
		}
	});

	callbacks::on(callbacks::type::finish_options_dialog, []()
	{
		if(display::ready) display::can_render_text = true;
	});

	callbacks::on(callbacks::type::do_options_dialog, []()
	{
		if (display::ready) display::can_render_text = false;
	});

	callbacks::on(callbacks::type::do_to_menu, []()
	{
		if (display::ready) display::can_render_text = false;
	});

	callbacks::on_begin_shot([](auto, auto)
	{
		if (display::ready) display::items_timeout = 200.0f;
	});
}

void display::update()
{
	if (display::ready)
	{
		if (display::can_render_text)
		{
			if (display::items_timeout < 200.0f)
			{

				Sexy::FloatingText_* header = (Sexy::FloatingText_*)Sexy::LogicMgr::AddStandardText(
							Sexy::Format("Items:"),
							110.0f,
							25.0f,
							14
				);
				header->unk_1 = 1;
				header->float_offset_start = 0.0f;
				header->color = 0x008D51;

				for (auto i = 0; i < 3; ++i)
				{
					Sexy::FloatingText_* item;

					if (player::item_inventory[i].name == "")
					{
						item = (Sexy::FloatingText_*)Sexy::LogicMgr::AddStandardText("---", 110.0f, 25.0f + (22.0f * (i + 1)), 14);
					}
					else
					{
						std::string text = Sexy::Format("%i: %s", (i + 1), player::item_inventory[i].name.c_str());

						if (player::item_inventory[i].count >= 2)
						{
							text = Sexy::Format("%i: %s x%i", (i + 1), player::item_inventory[i].name.c_str(), player::item_inventory[i].count);
						}

						item = (Sexy::FloatingText_*)Sexy::LogicMgr::AddStandardText(text, 110.0f, 25.0f + (22.0f * (i + 1)), 14);
						item->color = player::item_inventory[i].color;
					}

					item->unk_1 = 1;
					item->float_offset_start = 0.0f;
				}

				++display::items_timeout;
			}
		}
	}
}
