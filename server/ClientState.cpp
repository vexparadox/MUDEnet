#include "ClientState.hpp"

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