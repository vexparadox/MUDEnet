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
	for(int item_id : m_item_ids)
	{
		Item* item = item_manager.item_for_id(item_id);
		if(item)
		{
			ss << item->name() << " - " << item->description() << "\n";
		}
		else
		{
			std::cout << "Incorrect item found in inventory of ID " << item_id << std::endl;
		}
	}
	return ss.str();
}