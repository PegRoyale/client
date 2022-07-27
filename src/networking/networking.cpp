#include "networking.hpp"
#include "logger/logger.hpp"
#include "input/input.hpp"
#include "player/player.hpp"
#include "gameplay/gameplay.hpp"

bool networking::shutdown = false;
ENetAddress networking::address;
ENetHost* networking::client;
ENetPeer* networking::server;
bool networking::ready_up = false;
bool networking::wait_for_others = true;
std::vector<std::string> networking::player_list;
std::vector<int> networking::level_order;
std::string networking::room_name = "NEW_ROOM";
std::string networking::room_key = "KEY";
bool networking::server_alive = false;

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

	networking::update();
	networking::check_alive();
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

						std::string packet = logger::va("name=%s;roomid=%s;key=%s", player::username.c_str(),
							networking::room_name.c_str(), networking::room_key.c_str());

						networking::create_room(networking::room_name, networking::room_key);
						networking::send_packet(proto_t::NEW_USER, packet);
						networking::send_packet(proto_t::GET_USER_LIST);
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

			SetWindowTextA(input::hwnd, logger::va("PegRoyale | Room: %s | Players: %i", networking::room_name.c_str(), networking::player_list.size()).c_str());

			std::this_thread::sleep_for(16ms);
		}
	}).detach();
}

void networking::disconnect()
{
	networking::shutdown = true;
	enet_peer_disconnect(server, 0);
	//enet_host_destroy(client);
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

			case proto_t::NAME_CHANGE:
			{
				std::string new_name;

				for (auto i = 1; i < split_packet.size(); ++i)
				{
					if (split_packet[i].find("name") != std::string::npos)
					{
						new_name = logger::split(split_packet[i], "=")[1];
						continue;
					}
				}

				player::username = new_name;
			} break;

			case proto_t::GET_USER_LIST:
			{
				networking::player_list.clear();

				for (auto i = 1; i < split_packet.size(); ++i)
				{
					auto user = logger::split(split_packet[i], "=")[1];
					networking::player_list.emplace_back(user);
				}


			} break;

			case proto_t::ALREADY_IN_GAME:
			{
				networking::send_packet(proto_t::DIED);
				MessageBoxA(nullptr, "This room already has a match in session!", "PegRoyale", 0);
				exit(0);
			} break;

			case proto_t::GET_LEVEL_LIST:
			{
				for (auto i = 1; i < split_packet.size(); ++i)
				{
					auto level = std::stoi(logger::split(split_packet[i], "=")[1]);
					networking::level_order.emplace_back(level);
				}
			} break;

			case proto_t::ROOMS_FULL:
			{
				networking::shutdown = true;
				MessageBoxA(nullptr, "Maximum number of rooms reached on the server!", "PegRoyale", 0);
				exit(0);
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

			case proto_t::CHECK_SERVER_ALIVE:
			{
				networking::server_alive = true;
			} break;

			case proto_t::GRANT_WINNER:
			{
				/*std::string winner = "UNKNOWN";

				if (networking::player_list.size() == 1)
				{
					networking::ready_up = false;
					networking::wait_for_others = true;
					gameplay::prompted_leave = true;
					Sexy::ThunderballApp::DoToMenu();
					return;
				}

				for (auto i = 1; i < split_packet.size(); ++i)
				{
					if (split_packet[i].find("winner") != std::string::npos)
					{
						winner = logger::split(split_packet[i], "=")[1];
						continue;
					}
				}

				networking::ready_up = false;
				networking::wait_for_others = true;
				gameplay::prompted_leave = true;
				MessageBoxA(0, logger::va("%s has won the match!", winner.c_str()).c_str(), "PegRoyale", 0);
				Sexy::ThunderballApp::DoToMenu();*/
			} break;
		}
	}
}

void networking::check_alive()
{
	networking::server_alive = false;
	networking::send_packet(proto_t::CHECK_SERVER_ALIVE);
}

void networking::reset()
{
	networking::level_order.clear();
}
