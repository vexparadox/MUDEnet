#include "ClientManager.hpp"
#include "ClientState.hpp"
#include <iostream>
#include <thread>
#include <fstream>
#include "../shared/json.hpp"
#include <ctime>

using json = nlohmann::json;

namespace
{
	void save_internal(std::vector<ClientState> states)
	{
		json client_save_file;
	    client_save_file["version"] = CLIENT_SAVE_VERSION;
	    client_save_file["time"] = std::time(nullptr);
	    json client_array;
	    for(ClientState& state : states)
	    {
	        json client_data;
	        client_data["id"] = state.ID();
	        client_data["username"] = state.Username();
			client_data["password"] = state.Password();
	        client_data["location"] = state.LocationID();
	        client_array.push_back(client_data);
	    }
	    client_save_file["clients"] = client_array;

	    std::string filename("client_data.json");
	    std::ofstream file(filename);
	    if(file.is_open())
	    {
	    	file << client_save_file;
	    	file.close();
	    	std::cout << "Client data saved!" << std::endl;
	    }
	    else
	    {
	    	std::cout << "Client data file failed to open!" << std::endl;
	    }
	}
}

ClientManager::~ClientManager()
{
	for(ClientState* state : client_states)
	{
		delete state;
	}
	client_states.clear();
}

bool ClientManager::load_save(const std::string& filename)
{
	if(client_states.empty() == false)
	{
		std::cout << "Trying to load save when there are already clients, this isn't supported!" << std::endl;
		return false;
	}

	json json_obj;
	std::ifstream file(filename.c_str());
	if(file.is_open() == false)
	{
		std::cout << "Couldn't find save file of the name " << filename << std::endl;
		return false;
	}
	file >> json_obj;
	
	int version = json_obj["version"];
	if(version != CLIENT_SAVE_VERSION)
	{
		std::cout << "Loading different version file! Expecting " << CLIENT_SAVE_VERSION << " but got " << version << std::endl;
	}
	auto client_array = json_obj["clients"];
	for(auto client : client_array)
	{
		ClientState* new_state = new ClientState(client["id"], client["username"], client["password"]);
		new_state->SetLocation(client["location"]);
		client_states.push_back(new_state);
	}

	std::cout << "Loaded in " << client_states.size() << " clients" << std::endl;
	return true;
}

void ClientManager::print_users()
{
	std::cout << std::endl << "Online Users: " << std::endl;
    for(ClientState* state : client_states)
    {
        if(state && state->Peer())
        {
            std::cout << state->Username() << std::endl;
        }
    }

    std::cout << "Offline Users: " << std::endl;
    for(ClientState* state : client_states)
    {
        if(state && state->Peer() == nullptr)
        {
            std::cout << state->Username() << std::endl;
        }
    }
    std::cout << std::endl;
}

void ClientManager::save_state()
{	
	//make a copy of the client data so we can thread the saving
	std::vector<ClientState> states;
	for(ClientState* client_ptr : client_states)
	{
		states.push_back(*client_ptr);
	}
	//start a save thread
	std::thread save_thread(save_internal, states);
	save_thread.detach();
}

ClientState* ClientManager::register_new_client(ENetEvent* event, std::string username, std::string password)
{
	ClientState* new_client = new ClientState(client_states.size(), username, password);
	event->peer->data = malloc(sizeof(char));
	*(char*)event->peer->data = (char)new_client->ID();
	client_states.push_back(new_client);
	return new_client;
}

std::vector<ClientState*> ClientManager::online_users()
{
	std::vector<ClientState*> users;
	for(ClientState* client_ptr : client_states)
	{
		if(client_ptr->Peer())
		{
			users.push_back(client_ptr);
		}
	}
	return users;
}


ClientState* ClientManager::client_for_id(void* data)
{
	if(data)
	{
		//cast data point from an enet peer
		char user_id = *(char*)data;
		return client_for_id(user_id);
	}
	return nullptr;
}

ClientState* ClientManager::client_for_username(const std::string& username)
{
	auto found_client = std::find_if(client_states.begin(), client_states.end(), [username](ClientState* state)
	{
		return state->Username() == username;
	});
	if(found_client != client_states.end())
	{
		return (*found_client);
	}
	return nullptr;
}

ClientState* ClientManager::client_for_id(char user_id)
{
	auto found_client = std::find_if(client_states.begin(), client_states.end(), [user_id](ClientState* state)
	{
		return state->ID() == user_id;
	});
	if(found_client != client_states.end())
	{
		return (*found_client);
	}
	return nullptr;
}