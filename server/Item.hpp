#pragma once
#include <string>
#include <vector>
#include "../shared/json.hpp"

using json = nlohmann::json;

enum class ITEM_ATTRIBUTE : int
{
	WEAPON,
	NUM
};

class Item
{
	int m_id;
	std::vector<ITEM_ATTRIBUTE> m_attributes;
	std::string m_name;
	std::string m_description;
public:
	Item(json&); // load from file

	int ID() const { return m_id; }
	const std::string& name() const { return m_name; }
	const std::string& description() const { return m_description; }
};

ITEM_ATTRIBUTE item_attribute_from_string(const std::string&);