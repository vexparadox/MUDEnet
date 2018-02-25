#include "WorldState.hpp"
#include <iostream>
#include <fstream>
#include "ClientState.hpp"

void WorldState::load(const std::string& filename)
{
	std::ifstream i(filename.c_str());
	i >> json_obj;
}

void WorldState::parse()
{
	m_world_height = json_obj["height"];
	m_world_width = json_obj["width"];
	m_welcome_string = json_obj["welcome_text"];
	auto world_array = json_obj["worldmap"];
	for(auto world : world_array)
	{
		m_locations.emplace_back(Location(world["id"], world["title"], world["description"], world["here"], world["n"], world["e"], world["s"], world["w"], world["passable"]));
	}
}

bool Location::IsPassable(ClientState* client_state) const
{
	//check client has required items etcetc
	return client_state && m_passable;
}