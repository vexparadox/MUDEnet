#include "Model.hpp"

bool Model::Initialise()
{
    if(m_item_manager.load("items.json") == false || 
    m_quest_manager.load("quests.json") == false || 
    m_world_state.load("map.json") == false )
    {
        return false;
    }
    m_client_manager.load_save("client_data.json");

    m_actions.emplace_back(Action{&Model::look, "look", 0});
    m_actions.emplace_back(Action{&Model::say, "say", 1});
    m_actions.emplace_back(Action{&Model::go, "go", 1});
    m_actions.emplace_back(Action{&Model::help, "help", 0});
    m_actions.emplace_back(Action{&Model::inv, "inv", 0});
    m_actions.emplace_back(Action{&Model::pickup, "pickup", 1});
    m_actions.emplace_back(Action{&Model::quests, "quest", 1});

    return true;
}

void Model::Save()
{
    m_client_manager.save_state();
}

std::string Model::ProcessAction(ClientState& client, const std::vector<std::string>& tokens)
{
    auto found_action = std::find_if(m_actions.begin(), m_actions.end(), [&tokens](const Action& action)
    {
        return tokens.front() == action.m_trigger;
    });

    if(found_action != m_actions.end())
    {
        if(tokens.size()-1 >= found_action->m_min_num_args)
        {
            //call the matchings action with the tokens we've seperated
            ActionFPtr func = found_action->m_func;
            return (this->*func)(client, tokens);
        }
        else
        {
             return "This action needs parameters, try using help!";
        }
        
    }
    return "This is an unknown action, try using help!";
}

//look around the players current position as stored in their client state
//sends the player strings describing their current location and directions relative to it
std::string Model::look(ClientState& client_state, const std::vector<std::string>& tokens)
{
    const Location& client_loc = m_world_state.location(client_state.location_id());
    std::stringstream ss;
    ss << "\n ----" << client_loc.m_title << "----" << "\n";
    if(tokens.size() < 2)
    {
        ss << client_loc.m_description << "\n ----------------------- \n";
        ss << client_loc.m_here;
    }
    else
    {
        const std::string& param1 = tokens.at(1);
        const DIRECTION direction = direction_for_string(param1);
        if(direction != DIRECTION::NUM)
        {
            ss << client_loc.direction_string(direction);
        }
        else
        {
            ss << client_loc.m_here;
        }
    }
    ss << "\n";
    return ss.str();
}

//allows players to talk to people at the same location to them
std::string Model::say(ClientState& client_state, const std::vector<std::string>& tokens)
{
    std::stringstream ss;
    ss << client_state.username() << ":";
    //reconstruct from tokens, tbh this could be a lot better...
    for(int i = 1; i < tokens.size(); ++i)
    {
        ss << " " << tokens.at(i);
    }
    //std::cout << ss.str() << std::endl;
    for(ClientState* state : m_client_manager.online_users())
    {
        if(state->enet_peer() && client_state.location_id() == state->location_id())
        {
     //       message_peer(*state, ss.str());
        }
    }
    return ss.str();
}

//allows the players to move in n,e,s,w directions
//checks if in the map, if passable, and then applies the move their client state
std::string Model::go(ClientState& client_state, const std::vector<std::string>& tokens)
{
    const Location& client_loc = m_world_state.location(client_state.location_id());
    const std::string invalid_direction = "You can't go in that direction";
    std::string response;
    switch(direction_for_string(tokens.at(1)))
    {
        case DIRECTION::NORTH:
        {
            if(client_state.location_id() < m_world_state.width())
            {
                response = invalid_direction;
            }
            else if (m_world_state.location(client_state.location_id()-m_world_state.width()).IsPassable(client_state) == false)
            {
                response = "That region is impassable.";
            }
            else
            {
                client_state.set_location(client_state.location_id()-m_world_state.width());
                response = "You travel north.";
            }
            break;
        }
        case DIRECTION::EAST:
        {
            if((client_state.location_id()+1) % m_world_state.width() == 0)
            {
                response = invalid_direction;
            }
            else if (m_world_state.location(client_state.location_id()+1).IsPassable(client_state) == false)
            {
                response = "That region is impassable.";
            }
            else
            {
                client_state.set_location(client_state.location_id()+1);
                response = "You travel east.";
            }
            break;
        }
        case DIRECTION::SOUTH:
        {
            if((client_state.location_id() / m_world_state.height()) >= m_world_state.height())
            {
                response = invalid_direction;
            }
            else if (m_world_state.location(client_state.location_id()+m_world_state.width()).IsPassable(client_state) == false)
            {
                response = "That region is impassable.";
            }
            else
            {
                client_state.set_location(client_state.location_id()+m_world_state.width());
                response = "You travel south.";
            }
            break;
        }
        case DIRECTION::WEST:
        {
            if(client_state.location_id() % m_world_state.width() == 0)
            {
                response = invalid_direction;
            }
            else if (m_world_state.location(client_state.location_id()-1).IsPassable(client_state) == false)
            {
                response = "That region is impassable.";
            }
            else
            {
                client_state.set_location(client_state.location_id()-1);
                response = "You travel west.";
            }
            break;
        }
        default:
        {
            response = invalid_direction;
            break;
        }
    }
    //todo: we may want to tell other players in the location that this player has left/joined
    return response;
}

//prints a bunch of commands, should probably just be in data somewhere but eh
std::string Model::help(ClientState& client_state, const std::vector<std::string>&)
{
    std::stringstream ss;
    ss << "\n";
    ss << "You can use the following commands:" << "\n";
    ss << "go <direction>                   -  Go in a direction given" << "\n";
    ss << "look (direction)                 -  Look around, direction optional" << "\n";
    ss << "say <message>                    -  Speak to the people at your location" << "\n";
    ss << "quest <list/accept/abandon/complete> (id) -  List/accept/abandon/complete quests" << "\n";
    ss << "inv                              -  Check your inventory" << "\n";
    ss << "pickup <id>                      -  Pickup an item" << "\n";
    ss << "exit                             -  Logout";

    return ss.str();
}

std::string Model::inv(ClientState& client_state, const std::vector<std::string>&)
{
    return client_state.inventory().print_string(m_item_manager);
}

std::string Model::quests(ClientState& client_state, const std::vector<std::string>& tokens)
{
    //get the client's location
    const Location& client_loc = m_world_state.location(client_state.location_id());
    std::stringstream ss;

    //if we're listing quests
    if(tokens.at(1) == "list")
    {
        //get the client's active quests
        ss << client_state.quest_status_string(m_quest_manager, false);
        ss << "---- Available Quests ----\n";
        //get the available quests at their location
        auto available = client_loc.available_quests(client_state);
        if(available.empty())
        {
            ss << "There are no quests at this location.\n";
        }
        else
        {
            //create the strings for each available quest
            for(int quest_id : available)
            {
                Quest* quest = m_quest_manager.quest_for_id(quest_id);
                if(quest)
                {
                    ss << quest->quest_string(true);
                }
            }
        }
    }
    else
    {   
        //we need 3 arguments for accept/abandon
        if(tokens.size() < 3)
        {
            return "This action needs parameters, try using help!";
        }

        //get the 3rd argument as an int
        std::istringstream int_ss(tokens.at(2));
        if(int_ss.fail())
        {
            return "That wasn't an ID! Try giving numbers";
        }
        int quest_id;
        int_ss >> quest_id;

        //accept a quest at this location
        if(tokens.at(1) == "accept")
        {
            //check if it's available
            if(client_loc.is_quest_available(client_state, quest_id))
            {
                //accept if it is
                client_state.accept_quest(quest_id);
                //construct strings for acceptance
                ss << "Quest accepted!\n";
                Quest* quest = m_quest_manager.quest_for_id(quest_id);
                if(quest)
                {
                    ss << quest->accept_string() << "\n";
                }
            }
            else
            {
                //otherwise say it's not available
                ss << "That quest isn't available here!\n";
            }
        }
        else if(tokens.at(1) == "abandon")
        {
            if(client_state.has_active_quest(quest_id))
            {
                client_state.abandon_quest(quest_id);
                ss << "Quest abandoned.\n";
            }
            else
            {
                ss << "You're not currently on that quest.\n";
            }
        }
        else if(tokens.at(1) == "complete")
        {
            Quest* quest = m_quest_manager.quest_for_id(quest_id);
            if(quest && client_state.has_active_quest(*quest))
            {
                switch(m_quest_manager.completion_status(*quest, client_state, m_world_state.location(client_state.location_id())))
                {
                    case QUEST_COMPLETION_STATUS::COMPLETE:
                    {
                        client_state.complete_quest(*quest);
                        ss << quest->complete_string() << "\n";
                        break;
                    }
                    case QUEST_COMPLETION_STATUS::MISSING_ITEMS:
                    {
                        ss << "You're missing a required item to complete that quest.\n";
                        break;
                    }
                    case QUEST_COMPLETION_STATUS::BAD_LOCATION:
                    {
                        ss << "You're not in the right place to complete that quest.\n";
                        break;
                    }
                    default:
                    {
                        ss << "Unknown issue, see admin.\n";
                        break;
                    }
                }
            }
            else
            {
                ss << "You're not currently on that quest.\n";
            }
        }
    }
    return ss.str();
}

std::string Model::pickup(ClientState& client_state, const std::vector<std::string>& tokens)
{
    //get the 2nd argument as an int
    std::istringstream int_ss(tokens.at(1));
    if(int_ss.fail())
    {
        return "That wasn't an ID! Try giving numbers";
    }
    int item_id;
    int_ss >> item_id;
    std::stringstream ss;

    const Item* item = GetItemManager().item_for_id(item_id);
    const std::vector<int>& available = m_world_state.location(client_state.location_id()).m_available_items;
    const bool exists_at_location = item && std::find(available.begin(), available.end(), item_id) != available.end();

    if(exists_at_location)
    {
        if(item->Maximum() == 0 || client_state.inventory().num_items(*item) < item->Maximum())
        {
            client_state.inventory().gain_item(item_id);
            return "You picked up the " + item->name() + ".\n";
        }
        else
        {
            return "You can't carry anymore " + item->name() + ".\n";
        }
        
    }
    else if(item)
    {
        return "You can't see that item here.\n";
    }
    

    for(int available_item_id : m_world_state.location(client_state.location_id()).m_available_items)
    {
        if(available_item_id == item_id)
        {
            client_state.inventory().gain_item(item_id);
            return "You picked up the item.\n";
        }
    }
}