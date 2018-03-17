#pragma once
#include <string>
#include <vector>
#include "../shared/json.hpp"

class ClientState;

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
	std::vector<std::string> m_required_items;
	std::vector<int> m_available_quests;
	bool m_passable;
	Location(json&);

	bool IsPassable(ClientState*) const;
};

// I'm a naughty boi boi
class WorldState
{
	std::vector<Location> m_locations;
	std::string m_welcome_string;
	int m_world_height;
	int m_world_width;
public:
	int height() const { return m_world_height; }
	int width() const { return m_world_width; }
	const std::string& welcome_string() const { return m_welcome_string; }
	const Location& location(int location_id) const { return m_locations.at(location_id); }
	const std::vector<Location>& locations() { return m_locations; };
	bool load(const std::string& filename);
};