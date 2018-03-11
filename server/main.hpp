#pragma once
#include "ClientState.hpp"
#include "WorldState.hpp"
#include "ClientManager.hpp"
#include "ItemManager.hpp"
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

//type def functions 
typedef void (* Action)(ENetEvent* event);
typedef void (* MUDAction)(ENetEvent* event, std::vector<std::string>); 

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
Action actions[] = {
	message_recieved,
	new_user
};

//MUD code
WorldState world_state;
ClientManager client_manager;

std::vector<std::pair<std::string, MUDAction>> mud_actions;

void message_peer(ENetPeer* peer, const std::string& str);
void mud_look(ENetEvent* event, std::vector<std::string>);
void mud_say(ENetEvent* event, std::vector<std::string>);
void mud_go(ENetEvent* event, std::vector<std::string>);
void mud_help(ENetEvent* event, std::vector<std::string>);

