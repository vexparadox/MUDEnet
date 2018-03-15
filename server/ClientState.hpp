#pragma once
#include <string>
#include <enet/enet.h>
#include "Inventory.hpp"

using json = nlohmann::json;
class ClientState
{
private:
	const std::string m_username;
	const std::string m_password;
	const int m_id = -1;
	int m_locationid = 0;

	Inventory m_inventory;

	ENetPeer* m_enet_peer = nullptr; // set only when the user is connected
public:
	ClientState(){};
	ClientState(int id, const std::string& username, const std::string& password) 
	: m_id(id), m_username(username), m_password(password) {};

	void SetENetPeer(ENetPeer* peer) { m_enet_peer = peer; }
	ENetPeer* Peer() { return m_enet_peer; }

	void save(json&) const;

	const std::string& Username() const { return m_username; }
	const std::string& Password() const { return m_password; }
	int ID() const { return m_id; }
	int LocationID() const {return m_locationid;}
	void SetLocation(int l) { m_locationid = l;}
};