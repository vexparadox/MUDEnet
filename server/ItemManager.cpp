#include "ItemManager.hpp"
#include <iostream>
#include <fstream>
#include "../shared/json.hpp"

using json = nlohmann::json;

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
	auto item_array = json_obj["items"];
	for(auto item : item_array)
	{	
		//get a list of sttributes
		std::vector<ITEM_ATTRIBUTE> attributes;
		for(std::string attribute : item["attributes"])
		{
			ITEM_ATTRIBUTE found_attribute = item_attribute_from_string(attribute);
			if(found_attribute != ITEM_ATTRIBUTE::NUM)
			{
				attributes.push_back(found_attribute);
			}
		}
		//create a list of items
		Item* new_item = new Item(item["id"], item["name"], item["description"], std::move(attributes));
		m_items.push_back(new_item);
		m_item_map[new_item->ID()] = new_item;
	}
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

//returns an item attribute enum from a string
ITEM_ATTRIBUTE item_attribute_from_string(const std::string& str)
{
	static const std::unordered_map<std::string, ITEM_ATTRIBUTE> attributes =
	{{ "weapon", ITEM_ATTRIBUTE::WEAPON}};

	auto found_attribute = attributes.find(str);
	if(found_attribute != attributes.end())
	{
		return found_attribute->second;
	}
	std::cout << "Invalid ITEM_ATTRIBUTE found for string: " << str << std::endl;
	return ITEM_ATTRIBUTE::NUM;
}