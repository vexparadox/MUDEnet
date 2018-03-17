#include "ClientState.hpp"
#include "Quest.hpp"
ClientState::ClientState(json& client_obj) : m_inventory(client_obj["inventory"])
{
	m_id = client_obj["id"];
	m_username = client_obj["username"];
	m_password = client_obj["password"];
	m_locationid = client_obj["location"];
    for(auto quest : client_obj["active_quests"])
    {
        m_active_quests.push_back(quest);
    }

    for(auto quest : client_obj["completed_quests"])
    {
        m_completed_quests.push_back(quest);
    }
}

void ClientState::save(json& client_obj) const
{
    client_obj["id"] = ID();
    client_obj["username"] = username();
	client_obj["password"] = password();
    client_obj["location"] = location_id();
    json inv_obj;
    m_inventory.save(inv_obj);
    client_obj["inventory"] = inv_obj;
    client_obj["active_quests"] = m_active_quests;
    client_obj["completed_quests"] = m_completed_quests;
}

bool ClientState::has_completed_quest(int quest_id) const
{
    return std::find(m_completed_quests.begin(), m_completed_quests.end(), quest_id) != m_completed_quests.end();
}

bool ClientState::has_completed_quest(const Quest& quest) const
{
    return has_completed_quest(quest.ID());
}