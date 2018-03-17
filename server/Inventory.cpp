#include "Inventory.hpp"
#include "ItemManager.hpp"
#include <sstream>
void Inventory::save(json& inv_obj) const
{
	inv_obj["cash"] = cash();
	inv_obj["item_ids"] = m_item_ids;
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
			ss << item->Name() << "\n";
			ss << item->Description() << "\n";
		}
	}
	return ss.str();
}