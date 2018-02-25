#pragma once
#include "ClientState.hpp"
#include "WorldState.hpp"
#include "../shared/DataStream.hpp"
#include <memory>
#include <vector>
#include <stdio.h>
#include <iostream>
#include <cstring>
#include <enet/enet.h>
#include <atomic>
#include <thread>

//type def functions 
typedef void (* Action)(ENetEvent* event);
typedef void (* MUDAction)(ENetEvent* event, std::vector<std::string>); 

ENetAddress serverAddress;
std::atomic<ENetHost*> host; // the Enet Host
std::atomic<bool> run; //the running bool

//Enet actions
char clientCount; // the current count of users
DataStream broadcast_stream(1024);
void takeInput();
void messageRecieved(ENetEvent* event);
void newUser(ENetEvent* event);
void userDisconnected(ENetEvent* event);
void sendBroadcast();
//a list of actions
Action actions[] = {
	messageRecieved,
	newUser
};

//MUD code
WorldState world_state;
ClientState* ClientStateForID(char id);
std::vector<ClientState> client_states;
std::vector<std::pair<const std::string&, MUDAction>> mud_actions;

void respond_to_sender(ENetEvent* event, const std::string& str);
void mud_look(ENetEvent* event, std::vector<std::string>);
void mud_say(ENetEvent* event, std::vector<std::string>);
void mud_go(ENetEvent* event, std::vector<std::string>);
void mud_help(ENetEvent* event, std::vector<std::string>);

