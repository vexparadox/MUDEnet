#pragma once
#include <string>
#include <enet/enet.h>

class ClientState
{
private:
	const std::string m_username;
	const int m_id = -1;
	int m_locationid = 0;
	ENetPeer* m_enet_peer = nullptr; // set only when the user is connected
public:
	ClientState(){};
	ClientState(int id, const std::string& username) : m_id(id), m_username(username) {};

	void SetEnetPeer(ENetPeer* peer) { m_enet_peer = peer; }
	ENetPeer* EnetPeer() { return m_enet_peer; }

	const std::string& Username() const {return m_username;}
	int ID() const { return m_id; }
	int LocationID() const {return m_locationid;}
	void SetLocation(int l) { m_locationid = l;}
};