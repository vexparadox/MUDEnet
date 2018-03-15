#pragma once
#include <string>
#include <vector>
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
	Item(int id, const std::string& name, const std::string& description, std::vector<ITEM_ATTRIBUTE>&& attributes)
	: m_id(id), m_name(name), m_description(description), m_attributes(attributes){};

	int ID() const { return m_id; }
	const std::string& Name() const { return m_name; }
	const std::string& Description() const { return m_description; }
};