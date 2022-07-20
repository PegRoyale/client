#pragma once

class display final
{
public:
	static void init();
	static void update();

	static bool ready;
	static bool can_render_text;

private:
	static float items_timeout;
};
