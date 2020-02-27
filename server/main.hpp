#pragma once
#include "../shared/Config.hpp"
#include "ClientState.hpp"
#include "WorldState.hpp"
#include "ClientManager.hpp"
#include "ItemManager.hpp"
#include "QuestManager.hpp"
#include "../shared/DataStream.hpp"
#include <vector>
#include <ctime>
#include <stdio.h>
#include <iostream>
#include <cstring>
#include <enet/enet.h>
#include <atomic>
#include <thread>
#include <string>
#include <sstream>
#include <algorithm>

#define CLIENT_VERSION_REQUIRED 0
#define MESSAGE_TYPE_SERVER_SHUTDOWN 2
#define MESSAGE_TYPE_BAD_LOGIN 3
#define MESSAGE_TYPE_BAD_CLIENT_VERSION 4


//type def functions 
using ActionFPtr =  void (*)(ENetEvent* event);
using MUDActionFPtr = void (*)(ClientState&, ENetEvent* event, std::vector<std::string>); 

ENetAddress serverAddress;
std::atomic<ENetHost*> host; // the Enet Host
std::atomic<bool> run; //the running bool

//Enet actions
void take_input();
void message_recieved(ENetEvent* event);
void new_user(ENetEvent* event);
void user_disconnected(ENetEvent* event);
void send_broadcast(const std::string&);
void notify_exit();

//the last time the game saved
unsigned long last_save_time = 0;
int save_interval_seconds = 60;

std::vector<std::pair<std::string, std::string>> command_history;

//a list of actions
ActionFPtr actions[] = {
	message_recieved,
	new_user
};

//managers
WorldState world_state;
ClientManager client_manager;
ItemManager item_manager;
QuestManager quest_manager;


struct MUDAction
{
	MUDActionFPtr m_func = nullptr;
	std::string m_trigger;
	int m_min_num_args = 0;
};


std::vector<MUDAction> mud_actions;

void message_peer(ENetPeer* peer, const std::string& str);
void message_peer(ENetPeer* peer, Byte);

void mud_look(ClientState&, ENetEvent*, std::vector<std::string>);
void mud_say(ClientState&, ENetEvent*, std::vector<std::string>);
void mud_go(ClientState&, ENetEvent* , std::vector<std::string>);
void mud_help(ClientState&, ENetEvent*, std::vector<std::string>);
void mud_inv(ClientState&, ENetEvent*, std::vector<std::string>);
void mud_quests(ClientState&, ENetEvent*, std::vector<std::string>);
void mud_pickup(ClientState&, ENetEvent*, std::vector<std::string>);

