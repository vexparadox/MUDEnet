#include "Quest.hpp"

Quest::Quest(json& quest_obj)
{
	m_id = quest_obj["id"];
	m_title = quest_obj["title"];
	m_description = quest_obj["description"];
	m_cash_reward = quest_obj["cash_reward"];
	m_complete_string = quest_obj["complete"];
	m_accept_string = quest_obj["accept"];
	for(auto item_reward : quest_obj["item_rewards"])
	{
		m_item_rewards.push_back(item_reward);
	}
}