#pragma once
#include <vector>
#include <unordered_map>
#include "Quest.hpp"
class ClientState;
struct Location;

enum class QUEST_COMPLETION_STATUS : int
{
	COMPLETE,
	MISSING_ITEMS,
	BAD_LOCATION,
	UNKNOWN,
	NUM
};

class QuestManager
{
	std::vector<Quest*> m_quests;
	std::unordered_map<int, Quest*> m_quest_map;
public:
	bool load(const std::string&);

	QUEST_COMPLETION_STATUS completion_status(const Quest& quest, const ClientState& client, const Location& loc) const;

	Quest* quest_for_id(int) const;
};