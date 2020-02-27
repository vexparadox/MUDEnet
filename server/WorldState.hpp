#pragma once
#include <string>
#include <vector>
#include <array>
#include "../shared/json.hpp"

class ClientState;

using json = nlohmann::json;

enum DIRECTION : char
{
	NORTH,
	EAST,
	SOUTH,
	WEST,
	NUM
};

DIRECTION direction_for_string(const std::string&);

struct Location
{
	int m_id;
	std::string m_title;
	std::string m_description;
	std::string m_here;
	std::vector<std::string> m_required_items;
	std::vector<int> m_available_quests;
	std::vector<int> m_available_items;
	std::array<std::string, DIRECTION::NUM> m_direction_descriptions;
	bool m_passable;
	Location(json&);
	std::vector<int> available_quests(const ClientState&) const;
	bool is_quest_available(const ClientState&, int quest_id) const;
	const std::string& direction_string(DIRECTION direction) const;
	bool IsPassable(const ClientState&) const;
};

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