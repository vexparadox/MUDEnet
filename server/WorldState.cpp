#include "WorldState.hpp"
#include <iostream>
#include <fstream>
#include "ClientState.hpp"
#include "../shared/json.hpp"

using json = nlohmann::json;

bool WorldState::load(const std::string& filename)
{
	json json_obj;
	std::ifstream file(filename.c_str());
	if(file.is_open() == false)
	{
		std::cout << "Failed to find world file!" << std::endl;
		return false;
	}
	file >> json_obj;
	
	m_world_height = json_obj["height"];
	m_world_width = json_obj["width"];
	m_welcome_string = json_obj["welcome_text"];
	auto world_array = json_obj["worldmap"];
	for(auto world : world_array)
	{
		std::vector<std::string> required_items;
		for(auto item : world["requireditems"])
		{
			required_items.push_back(item.get<std::string>());
		}
		m_locations.emplace_back(Location(world["id"], world["title"], world["description"], world["here"], world["n"], world["e"], world["s"], world["w"], std::move(required_items), world["passable"]));
	}
	return true;
}

bool Location::IsPassable(ClientState* client_state) const
{
	//check client has required items etcetc
	return client_state && m_passable;
}