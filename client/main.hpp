#pragma once
#include <enet/enet.h>
#include "../shared/DataStream.hpp"
#include <stdio.h>
#include <string.h>
#include <atomic>
#include <thread>

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


void serverClosed(ENetEvent* event);
void messageRecieved(ENetEvent* event);
void uniqueID(ENetEvent* event);

//a list of actions
Action actions[] = {
	messageRecieved,
	uniqueID,
	serverClosed
};