#include "QuestManager.hpp"
#include "ClientState.hpp"
#include "WorldState.hpp"
#include <fstream>
#include <iostream>
using json = nlohmann::json;

bool QuestManager::load(const std::string& filename)
{
	json json_obj;
	std::ifstream file(filename.c_str());
	if(file.is_open() == false)
	{
		std::cout << "Failed to find quests file!" << std::endl;
		return false;
	}
	file >> json_obj;
	
	for(auto quest : json_obj["quests"])
	{
		Quest* new_quest = new Quest(quest);
		m_quests.push_back(new_quest);
		m_quest_map[new_quest->ID()] = new_quest;
	}
	std::cout << "Quest file loaded correctly: " << filename << std::endl;
	return true;
}

Quest* QuestManager::quest_for_id(int id) const
{
	auto found_quest = m_quest_map.find(id);
	if(found_quest != m_quest_map.end())
	{
		return found_quest->second;
	}
	return nullptr;
}

QUEST_COMPLETION_STATUS QuestManager::completion_status(const Quest& quest, const ClientState* client, const Location& location) const
{
	if(client == nullptr)
	{
		return QUEST_COMPLETION_STATUS::UNKNOWN;
	}

	if(location.m_id != quest.required_location())
	{
		return QUEST_COMPLETION_STATUS::BAD_LOCATION;
	}

	const bool has_items = std::all_of(quest.required_items().begin(), quest.required_items().end(), [client](int item_id)
	{
		return client->inventory().has_item(item_id);
	});

	if(has_items == false)
	{
		return QUEST_COMPLETION_STATUS::MISSING_ITEMS;
	}

	return QUEST_COMPLETION_STATUS::COMPLETE;
}