#pragma once

enum class proto_t
{
	NONE = -1,
	CREATE_ROOM,
	NEW_USER,
	READY_UP,
	START_GAME,
};

class networking final
{
public:
	static void init();
	static void update();
	static void cleanup();
	static void send_packet(proto_t proto, const std::string& info = "");
	static void create_room(const std::string& roomid, const std::string& key);
	static void handle_packet(ENetPacket* packet, ENetPeer* peer);

	static bool shutdown;
	static bool ready_up;

private:
	static ENetAddress address;
	static ENetHost* client;
	static ENetPeer* server;
};
