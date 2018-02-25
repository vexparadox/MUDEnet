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
	Location(int id, const std::string& title, const std::string& description, const std::string& here, const std::string& north, const std::string& east, const std::string& south, const std::string& west)
	: m_id(id), m_title(title), m_description(description), m_here(here), m_north(north), m_east(east), m_south(south), m_west(west) {};
};

// I'm a naughty boi boi
class WorldState
{
	int m_world_height;
	int m_world_width;
	std::string m_welcome_string;
	std::vector<Location> m_locations;
public:
	const std::vector<Location>& Locations() { return m_locations; };
	json json_obj;
	void load(const std::string& filename);
	void parse();
};