#include "WorldState.hpp"
#include <iostream>
#include <fstream>
#include "ClientState.hpp"

DIRECTION direction_for_string(const std::string& str)
{
	//could be a static map
    if(str == "n" || str == "north")
    {
        return DIRECTION::NORTH;
    }
    else if(str == "e" || str == "east")
    {
        return DIRECTION::EAST;
    }
    else if(str == "s" || str == "south")
    {
        return DIRECTION::SOUTH;
    }
    else if(str == "w" || str == "west")
    {
        return DIRECTION::WEST;
    }
    return DIRECTION::NUM;
}

Location::Location(json& location_obj)
{
	m_id = location_obj["id"];
	m_title = location_obj["title"];
	m_description = location_obj["description"];
	m_here = location_obj["here"];
	m_direction_descriptions[DIRECTION::NORTH] = location_obj["n"];
	m_direction_descriptions[DIRECTION::EAST] = location_obj["e"];
	m_direction_descriptions[DIRECTION::SOUTH] = location_obj["s"];
	m_direction_descriptions[DIRECTION::WEST] = location_obj["w"];
	m_passable = location_obj["passable"];
	for(auto quest : location_obj["quests"])
	{
		m_available_quests.push_back(quest);
	}

	for(auto item : location_obj["items"])
	{
		m_available_items.push_back(item);
	}
}

bool Location::IsPassable(const ClientState&) const
{
	//todo: check client has required items etcetc
	return m_passable;
}

std::vector<int> Location::available_quests(const ClientState& client_state) const
{
	std::vector<int> quests;
	quests.reserve(m_available_quests.size());
	std::copy_if(m_available_quests.begin(), m_available_quests.end(), std::back_inserter(quests), [client_state](int quest_id)
	{
		return client_state.has_completed_quest(quest_id) == false && client_state.has_active_quest(quest_id) == false;
	});
	return quests;
}

bool Location::is_quest_available(const ClientState& client_state, int quest_id) const
{
	auto available = available_quests(client_state);
	return std::find(available.begin(), available.end(), quest_id) != available.end();
}

const std::string& Location::direction_string(DIRECTION direction) const
{
	static const std::string empty;
	if(direction != DIRECTION::NUM)
	{
		return m_direction_descriptions[direction];
	}
	return empty;
}

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
	for(auto location : json_obj["worldmap"])
	{
		m_locations.emplace_back(Location(location));
	}
	std::cout << "World loaded correctly: " << filename << std::endl;
	return true;
}