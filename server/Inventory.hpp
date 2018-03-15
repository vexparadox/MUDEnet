#pragma once
#include <vector>
#include "../shared/json.hpp"

using json = nlohmann::json;
class ItemManager;
class Inventory
{
	std::vector<int> m_item_ids;
	int m_cash = 0;
public:
	Inventory(){};
	Inventory(int cash, std::vector<int>&& item_ids) : m_cash(cash), m_item_ids(item_ids) {};
	
	int cash() const { return m_cash; }

	std::string print_string(const ItemManager&) const;

	void save(json&) const;
};