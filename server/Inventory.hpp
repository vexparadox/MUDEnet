#pragma once
#include <vector>
#include "../shared/json.hpp"

using json = nlohmann::json;
class ItemManager;
class Item;
class Inventory
{
	std::vector<int> m_item_ids;
	int m_cash = 0;
public:
	Inventory(json&); //load from file
	Inventory(){}; //default load
	Inventory(int cash, std::vector<int>&& item_ids) : m_cash(cash), m_item_ids(item_ids) {};
	
	int cash() const { return m_cash; }

	std::string print_string(const ItemManager&) const;

	void gain_cash(int cash) { m_cash += cash; }
	void gain_item(int item_id) { m_item_ids.push_back(item_id); }
	void gain_item(const Item&);
	void lose_item(int item_id);
	void lose_item(const Item&);
	bool has_item(int item_id) const;
	bool has_item(const Item&) const;
	int num_items(int item_id) const;
	int num_items(const Item&) const;

	void save(json&) const;
};