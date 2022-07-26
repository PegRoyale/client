#pragma once

enum class proto_t
{
	NONE = -1,
	CREATE_ROOM,
	NEW_USER,
	READY_UP,
	START_GAME,
	GET_USER_LIST,
	USE_POWEWRUP,
	DIED,
	NAME_CHANGE,
	GET_LEVEL_LIST,
	ROOMS_FULL,
	ALREADY_IN_GAME,
	CHECK_SERVER_ALIVE,
};

class networking final
{
public:
	static void init();
	static void update();
	static void disconnect();
	static void send_packet(proto_t proto, const std::string& info = "");
	static void create_room(const std::string& roomid, const std::string& key);
	static void handle_packet(ENetPacket* packet, ENetPeer* peer);
	static void check_alive();

	static bool shutdown;
	static bool ready_up;
	static bool wait_for_others;
	static std::vector<std::string> player_list;
	static std::vector<int> level_order;
	static std::string room_name;
	static std::string room_key;
	static bool server_alive;

private:
	static ENetAddress address;
	static ENetHost* client;
	static ENetPeer* server;
};
