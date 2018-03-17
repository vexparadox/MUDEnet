#pragma once
#include <string>
#include <enet/enet.h>
#include "Inventory.hpp"

using json = nlohmann::json;
class Quest;
class QuestManager;
class ClientState
{
private:
	std::string m_username;
	std::string m_password;
	int m_id = -1;
	int m_locationid = 0;

	Inventory m_inventory;

	std::vector<int> m_completed_quests;
	std::vector<int> m_active_quests;

	ENetPeer* m_enet_peer = nullptr; // set only when the user is connected
public:
	ClientState(json&); // load from file
	ClientState(int id, const std::string& username, const std::string& password) 
	: m_id(id), m_username(username), m_password(password) {};

	void set_enet_peer(ENetPeer* peer) { m_enet_peer = peer; }
	ENetPeer* enet_peer() { return m_enet_peer; }
	void save(json&) const;

	const Inventory& inventory() const { return m_inventory; }
	const std::string& username() const { return m_username; }
	const std::string& password() const { return m_password; }
	std::string quest_status_string(const QuestManager&, bool all_quests) const;
	void accept_quest(int quest_id);
	void accept_quest(const Quest&);
	bool has_completed_quest(int quest_id) const;
	bool has_completed_quest(const Quest&) const;
	bool has_active_quest(const Quest&) const;
	bool has_active_quest(int quest_id) const;
	int ID() const { return m_id; }
	int location_id() const {return m_locationid;}
	void set_location(int l) { m_locationid = l;}
};