#pragma once

class gameplay final
{
public:
	static void init();
	static void reset();

	static int balls_left;

private:
	static void purple_peg_hit();
	static void green_peg_hit();
	static Sexy::PhysObj* last_phys_obj;
	static Sexy::Ball* last_ball;
};
