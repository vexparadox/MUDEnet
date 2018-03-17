#include "ClientState.hpp"

ClientState::ClientState(json& client_obj) : m_inventory(client_obj["inventory"])
{
	m_id = client_obj["id"];
	m_username = client_obj["username"];
	m_password = client_obj["password"];
	m_locationid = client_obj["location"];
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
}