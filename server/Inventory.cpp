#include "Inventory.hpp"

void Inventory::save(json& inv_obj) const
{
	inv_obj["cash"] = cash();
	inv_obj["item_ids"] = m_item_ids;
}