#include "ClientState.hpp"

void ClientState::save(json& client_obj) const
{
    client_obj["id"] = ID();
    client_obj["username"] = Username();
	client_obj["password"] = Password();
    client_obj["location"] = LocationID();
    json inv_obj;
    m_inventory.save(inv_obj);
    client_obj["inventory"] = inv_obj;
}