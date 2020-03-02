#include "Inventory.hpp"
#include "ItemManager.hpp"
#include <sstream>
#include <iostream>

void Inventory::save(json& inv_obj) const
{
	inv_obj["cash"] = cash();
	inv_obj["item_ids"] = m_item_ids;
}

Inventory::Inventory(json& inv_obj)
{
	for(auto item : inv_obj["item_ids"])
	{
		m_item_ids.push_back(item);
	}
	m_cash = inv_obj["cash"];	
}

std::string Inventory::print_string(const ItemManager& item_manager) const
{
	std::stringstream ss;
	ss << "---- Inventory ----" << "\n";
	ss << "Cash: " << cash() << "\n";
	ss << "--- Items ---" << "\n";

	//sort and get uniques
	std::vector<int> items = m_item_ids;
	std::sort(items.begin(), items.end());
	items.erase(std::unique(items.begin(), items.end()), items.end());

	for(int item_id : items)
	{
		Item* item = item_manager.item_for_id(item_id);
		if(item)
		{
			if(item->Maximum() != 0)
			{
				ss << "[" << num_items(*item) << "/" << item->Maximum() << "] ";
			}
			ss << item->name() << " - " << item->description() << "\n";
		}
		else
		{
			std::cout << "Incorrect item found in inventory of ID " << item_id << std::endl;
		}
	}
	return ss.str();
}

void Inventory::gain_item(const Item& item)
{
	gain_item(item.ID());
}

void Inventory::lose_item(int item_id)
{
	auto iter = std::find(m_item_ids.begin(), m_item_ids.end(), item_id);
	if(iter != m_item_ids.end())
	{
		m_item_ids.erase(iter);
	}
}

void Inventory::lose_item(const Item& item)
{
	lose_item(item.ID());
}

bool Inventory::has_item(int item_id) const
{
	return std::find(m_item_ids.begin(), m_item_ids.end(), item_id) != m_item_ids.end();
}

bool Inventory::has_item(const Item& item) const
{
	return has_item(item.ID());
}

int Inventory::num_items(int item_id) const
{
	return std::count(m_item_ids.begin(), m_item_ids.end(), item_id);
}

int Inventory::num_items(const Item& item) const
{
	return num_items(item.ID());
}