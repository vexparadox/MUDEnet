#include "Item.hpp"
#include <iostream>
#include <unordered_map>
Item::Item(json& item_obj)
{
	for(std::string attribute_string : item_obj["attributes"])
	{
		ITEM_ATTRIBUTE attribute = item_attribute_from_string(attribute_string);
		if(attribute != ITEM_ATTRIBUTE::NUM)
		{
			m_attributes.push_back(attribute);
		}
	}
	m_id = item_obj["id"]; 
	m_name = item_obj["name"];
	m_description = item_obj["description"];
	m_maximum_num = item_obj["max"];
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