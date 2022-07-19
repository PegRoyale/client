#pragma once

class player final
{
public:
	static void init();

	//General statistics to show the user
	static int levels_beat;
	static int shots_fired;
	static int pegs_hist;
	static int orange_pegs_hit;
	static int purple_pegs_hit;
	static int green_pegs_hit;
	static int powerups_used;
	static int buckets_sunk;
};
