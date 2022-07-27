#pragma once

class display final
{
public:
	static void init();
	static void update();

	static bool can_render_text;
	static bool ready;
	static bool show_hud;

private:
	static float items_timeout;
};
