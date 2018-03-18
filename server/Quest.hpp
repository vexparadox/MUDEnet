#pragma once
#include <string>
#include "../shared/json.hpp"
using json = nlohmann::json;

class Quest
{
	int m_id;
	std::string m_title;
	std::string m_description;
	std::string m_complete_string;
	std::string m_accept_string;
	std::vector<int> m_item_rewards;
	std::vector<int> m_required_items;
	int m_required_location;
	int m_cash_reward;
public:
	Quest(json&); // load from file

	int ID() const {return m_id;}
	const std::string& title() const { return m_title; }
	const std::string& description() const { return m_description; }
	const std::string& complete_string() const {return m_complete_string;}
	const std::string& accept_string() const { return m_accept_string; }
	int required_location() const {return m_required_location;}
	const std::vector<int>& required_items() const { return m_required_items; }
	int cash_reward() const { return m_cash_reward; }
	const std::vector<int>& item_rewards() const { return m_item_rewards; }

	std::string quest_string(bool show_rewards) const;
};