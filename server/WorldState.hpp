#pragma once
#include "json.hpp"

using json = nlohmann::json;

struct Location
{
	int m_id;
	std::string m_title;
	std::string m_description;
	std::string m_here;
	std::string m_north;
	std::string m_east;
	std::string m_south;
	std::string m_west;
	bool m_passable;
	Location(int id, const std::string& title, const std::string& description, const std::string& here, const std::string& north, const std::string& east, const std::string& south, const std::string& west, bool passable)
	: m_id(id), m_title(title), m_description(description), m_here(here), m_north(north), m_east(east), m_south(south), m_west(west), m_passable(passable){};
};

// I'm a naughty boi boi
class WorldState
{
	std::vector<Location> m_locations;
public:
	std::string m_welcome_string;
	int m_world_height;
	int m_world_width;
	
	const std::vector<Location>& Locations() { return m_locations; };
	json json_obj;
	void load(const std::string& filename);
	void parse();
};