#pragma once
#include <string>
#include "../shared/json.hpp"
using json = nlohmann::json;

class Quest
{
	int m_id;
	std::string m_title;
	std::string m_description;
	std::vector<int> m_item_rewards;
	int m_cash_reward;
public:
	Quest(json&); // load from file

	int ID() const {return m_id;}
	const std::string& title() const { return m_title; }
	const std::string& description() const { return m_description; }
	int cash_reward() const { return m_cash_reward; }
};