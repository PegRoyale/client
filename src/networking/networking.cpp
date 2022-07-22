#include "networking.hpp"
#include "logger/logger.hpp"
#include "input/input.hpp"
#include "player/player.hpp"

bool networking::shutdown = false;
ENetAddress networking::address;
ENetHost* networking::client;
ENetPeer* networking::server;
bool networking::ready_up = false;
bool networking::wait_for_others = true;
std::vector<std::string> networking::player_list;

void networking::init()
{
	networking::client = enet_host_create(0, 1, 2, 0, 0);

	if (!client)
	{
		PRINT_ERROR("Client is invalid");
		PRINT_ERROR("Shutting down (%i)", 0);
		networking::shutdown = true;
	}

	enet_address_set_host(&address, "71.45.142.6");
	address.port = 23363;
	server = enet_host_connect(client, &address, 2, 0);

	if (!server)
	{
		PRINT_ERROR("Failed to find server");
		PRINT_ERROR("Shutting down (%i)", 1);
		networking::shutdown = true;
	}

	callbacks::after_start_game([](auto this_)
	{
		if (!networking::ready_up)
		{
			networking::ready_up = true;
			networking::send_packet(proto_t::READY_UP);

			while (networking::wait_for_others) Sleep(150);
		}
	});

	networking::update();
}

void networking::update()
{
	std::thread([]()
	{
		while (!networking::shutdown)
		{
			ENetEvent evt;
			while (enet_host_service(client, &evt, 1000) > 0)
			{
				switch (evt.type)
				{
					case ENET_EVENT_TYPE_RECEIVE:
					{
						networking::handle_packet(evt.packet, evt.peer);
					} break;

					case ENET_EVENT_TYPE_CONNECT:
					{
						PRINT_INFO("Connected to the server");

						std::string packet = logger::va("name=%s;roomid=NEW_ROOM;key=KEY", player::username.c_str());

						networking::create_room("NEW_ROOM", "KEY");
						networking::send_packet(proto_t::NEW_USER, packet);
					} break;

					case ENET_EVENT_TYPE_DISCONNECT:
						PRINT_INFO("Disconnect from the server");
						if (!networking::shutdown)
						{
							MessageBoxA(nullptr, "Disconnected from the server!", "PegRoyale", 0);
							exit(0);
						}
						break;
					}
			}
			std::this_thread::sleep_for(16ms);
		}
	}).detach();
}

void networking::cleanup()
{
	networking::shutdown = true;
	enet_peer_disconnect(server, 0);
	enet_host_destroy(client);
}

void networking::send_packet(proto_t proto, const std::string& info)
{
	std::string final_info = logger::va("proto=%i;", proto).append(info);
	ENetPacket* packet = enet_packet_create(final_info.c_str(), final_info.size() + 1, ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(server, 0, packet);
	PRINT_DEBUG("Sent \"%s\" to the server", packet->data);
}

void networking::create_room(const std::string& roomid, const std::string& key)
{
	std::string packet = logger::va("roomid=%s;", roomid.c_str()).append(logger::va("key=%s;", key.c_str()));
	networking::send_packet(proto_t::CREATE_ROOM, packet);
}

void networking::handle_packet(ENetPacket* packet, ENetPeer* peer)
{
	if (peer != server)
	{
		PRINT_ERROR("Recieved request from unknown peer!");
		return;
	}

	auto split_packet = logger::split(std::string((char*)packet->data), ";");

	proto_t proto = proto_t::NONE;

	if (split_packet[0].find("proto") != std::string::npos)
	{
		proto = (proto_t)std::stoi(logger::split(split_packet[0], "=")[1]);
	}
	else
	{
		PRINT_ERROR("Unable to find protocol information!");
		return;
	}

	if (proto != proto_t::NONE)
	{
		switch (proto)
		{
			case proto_t::START_GAME:
			{
				networking::wait_for_others = false;
			} break;

			case proto_t::GET_USER_LIST:
			{
				for (auto i = 1; i < split_packet.size(); ++i)
				{
					auto user = logger::split(split_packet[i], "=")[1];
					networking::player_list.emplace_back(user);
				}
			} break;

			case proto_t::USE_POWEWRUP:
			{
				powerup_t powerup = powerup_t::TAKE_BALL;
				std::string user;

				for (auto i = 1; i < split_packet.size(); ++i)
				{
					if (split_packet[i].find("powerup") != std::string::npos)
					{
						powerup = (powerup_t)std::stoi(logger::split(split_packet[i], "=")[1]);
						continue;
					}
					else if (split_packet[i].find("user") != std::string::npos)
					{
						user = logger::split(split_packet[i], "=")[1];
						continue;
					}
				}

				player::handle_enemy_powerup(powerup, user);
			} break;
		}
	}
}
