#pragma once

class gameplay final
{
public:
	static void init();

	static int balls_left;

private:
	static void purple_peg_hit();
	static void green_peg_hit();
};
