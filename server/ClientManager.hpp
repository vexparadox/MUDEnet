#pragma once
#include <enet/enet.h>
#include <vector>
#include <string>

class ClientState;
#define CLIENT_SAVE_VERSION 1
class ClientManager
{
private:
	std::vector<ClientState*> client_states;
public:

	~ClientManager();
	ClientState* client_for_id(void*);
	ClientState* client_for_username(const std::string&);
	ClientState* client_for_id(char);
	std::vector<ClientState*> online_users();

	bool load_save(const std::string&);
	void save_state();
	void print_users();
	ClientState* register_new_client(ENetEvent* event, std::string username, std::string password);
};