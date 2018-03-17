#include "QuestManager.hpp"
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