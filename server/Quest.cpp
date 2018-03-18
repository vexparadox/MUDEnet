#include "Quest.hpp"
#include "ClientState.hpp"
#include "WorldState.hpp"
#include <sstream>
Quest::Quest(json& quest_obj)
{
	m_id = quest_obj["id"];
	m_title = quest_obj["title"];
	m_description = quest_obj["description"];
	m_cash_reward = quest_obj["cash_reward"];
	m_complete_string = quest_obj["complete"];
	m_accept_string = quest_obj["accept"];
	m_required_location = quest_obj["required_location"];
	for(auto item_reward : quest_obj["item_rewards"])
	{
		m_item_rewards.push_back(item_reward);
	}
	for(auto required_item : quest_obj["required_items"])
	{
		m_required_items.push_back(required_item);
	}
}

std::string Quest::quest_string(bool show_rewards) const
{
	std::stringstream ss;
    ss << "["<< ID() << "] "<< title() << " - " << description() << "\n";
    if(show_rewards)
	{
		ss << "Cash Reward: " << cash_reward() << "\n";
	}
	return ss.str();
}