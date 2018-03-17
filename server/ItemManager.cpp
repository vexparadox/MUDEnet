#include "ItemManager.hpp"
#include <iostream>
#include <fstream>

using json = nlohmann::json;

ItemManager::~ItemManager()
{
	m_item_map.clear();
	for(Item* item : m_items)
	{
		delete item;
	}
	m_items.clear();
}

bool ItemManager::load(const std::string& filename)
{
	json json_obj;
	std::ifstream file(filename.c_str());
	if(file.is_open() == false)
	{
		std::cout << "Failed to find items file!" << std::endl;
		return false;
	}
	file >> json_obj;
	for(auto item : json_obj["items"])
	{	
		Item* new_item = new Item(item);
		m_items.push_back(new_item);
		m_item_map[new_item->ID()] = new_item;
	}
	std::cout << "Item file loaded correctly: " << filename << std::endl;
	return true;
}

Item* ItemManager::item_for_id(int id) const
{
	auto found_item = m_item_map.find(id);
	if(found_item != m_item_map.end())
	{
		return found_item->second;
	}
	return nullptr;
}