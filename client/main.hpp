#pragma once
#include <enet/enet.h>
#include "../shared/DataStream.hpp"
#include "../shared/md5.hpp"
#include <stdio.h>
#include <string.h>
#include <atomic>
#include <thread>

#define CLIENT_VERSION 1

//type def functions 
typedef void (* Action)(ENetEvent* event);

std::atomic<ENetHost*> client;
std::atomic<ENetPeer*> peer;
std::atomic<ENetPeer*> server;
ENetAddress clientAddress;
bool connected = false; // if we're connected or not

char current_user_id = -1;

std::atomic<bool> running; // the running boolean

void disconnect();
void takeInput();
void getUsername();

//enet methods
void serverClosed(ENetEvent* event);
void messageRecieved(ENetEvent* event);
void uniqueID(ENetEvent* event);
void badLogin(ENetEvent* event);
void badClient(ENetEvent* event);

//a list of actions
Action actions[] = {
	messageRecieved,
	uniqueID,
	serverClosed,
	badLogin,
	badClient
};