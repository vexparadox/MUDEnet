#pragma once
#include "Item.hpp"
#include <vector>
#include <unordered_map>
#include <string>
class ItemManager
{
	std::vector<Item*> m_items;
	std::unordered_map<int, Item*> m_item_map;
public:
	~ItemManager();
	bool load(const std::string&);
	
	Item* item_for_id(int id) const;
};