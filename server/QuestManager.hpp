#pragma once
#include <vector>
#include <unordered_map>
#include "Quest.hpp"
class QuestManager
{
	std::vector<Quest*> m_quests;
	std::unordered_map<int, Quest*> m_quest_map;
public:
	bool load(const std::string&);

	Quest* quest_for_id(int) const;
};