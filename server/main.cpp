#include "main.hpp"
int main(int argc, char const *argv[])
{
    if (enet_initialize () != 0)
    {
        std::cout << "An error occurred while initializing ENet." << std::endl;
        return 1;
    }

    if(argc < 3)
    {
        std::cout << "No address or port was supplied." << std::endl;
    	return 1;
    }
    enet_address_set_host(&serverAddress, argv[1]);
    serverAddress.port = atoi(argv[2]);

    host.store(enet_host_create (&serverAddress, 32, 2, 0, 0));
    if (!host.load()) 
    { 
        std::cout << "An error occurred while trying to create the server host." << std::endl;
        return 1;
    }

    if(item_manager.load("items.json") == false || 
        quest_manager.load("quests.json") == false || 
        world_state.load("map.json") == false )
    {
        return 1;
    }
    client_manager.load_save("client_data.json");

    //deinit enet
    atexit (enet_deinitialize);

    //make the pairs of mud functions
    mud_actions.push_back(std::make_pair("look", mud_look));
    mud_actions.push_back(std::make_pair("say", mud_say));
    mud_actions.push_back(std::make_pair("go", mud_go));
    mud_actions.push_back(std::make_pair("help", mud_help));
    mud_actions.push_back(std::make_pair("inv", mud_inv));
    mud_actions.push_back(std::make_pair("pickup", mud_pickup));
    mud_actions.push_back(std::make_pair("quest", mud_quests));

    std::cout << "Server was started on " << argv[1] << ":" << argv[2] << std::endl;

    //start input thread
    std::thread inputThread(&take_input);

    //last save time as now
    last_save_time = std::time(nullptr);

    run.store(true);
    while(run.load()){
        ENetEvent event;
        //wait upto 15ms for an event
        while (enet_host_service (host.load(), &event, 15) > 0)
        {
            switch (event.type)
            {
            //When a player connects
            case ENET_EVENT_TYPE_CONNECT:
            {
                std::cout << std::endl << "A client connected from " << event.peer->address.host << ":" << event.peer->address.port << std::endl;
            }
            break;
            case ENET_EVENT_TYPE_RECEIVE:
            {
                //call the action that corrosponds with the first byte
                //see the READEME
                actions[event.packet->data[0]](&event);
            }break;
            case ENET_EVENT_TYPE_DISCONNECT:
                user_disconnected(&event);
                break;
            case ENET_EVENT_TYPE_NONE:
                break;
            }
        }
        //save every 60 seconds
        if(std::time(nullptr)-last_save_time > save_interval_seconds)
        {
            client_manager.save_state();
            last_save_time = std::time(nullptr);
        }
    }
    notify_exit();
    enet_host_destroy(host);
    inputThread.join();
	return 0;
}

void take_input()
{
    const int message_content_size = MSG_BUFFER_SIZE-2;
    char buffer[message_content_size];
    while (run.load()){
        DataStream stream(MSG_BUFFER_SIZE);
        memset(buffer, 0, message_content_size);
        fgets(buffer, message_content_size, stdin);
        //get rid of that pesky \n
        char* temp = buffer+strlen(buffer)-1;
        *temp = '\0';
        stream.write(Byte(0)); // this is a 0 action!
        stream.write(Byte(255)); // set the ID to 255, this is reserved for the server
        if(strcmp(buffer, "") != 0)
        {
            if(strcmp(buffer, "exit") == 0)
            {
                //save before exit
                client_manager.save_state();
        		run.store(false);
        	}
            else if(strcmp(buffer, "list") == 0)
            {
                client_manager.print_users();
            }
            else if(strcmp(buffer, "save") == 0)
            {
                client_manager.save_state();
            }
            else if(strcmp(buffer, "history") == 0)
            {
                std::cout << "======= Command History =======" << std::endl;
                for(std::pair<std::string, std::string>& pair : command_history)
                {
                    std::cout << pair.first << " - " << pair.second << std::endl;
                }
                std::cout << std::endl;
            }
            else
            {
                stream.write(buffer, message_content_size);
        	    ENetPacket* packet = enet_packet_create (stream.data(), stream.size(), ENET_PACKET_FLAG_RELIABLE);
			    enet_host_broadcast (host.load(), 0, packet);         
			    enet_host_flush (host.load());	
			    printf("\033[1A"); //go up one line
			    printf("\033[K"); //delete to the end of the line
			    printf("\rServer: %s\n", stream.data()+2); // use \r to get back to the start and print
        	}
        }

    }
}

void notify_exit()
{
    DataStream broadcast_stream(1);
    broadcast_stream.write(Byte(MESSAGE_TYPE_SERVER_SHUTDOWN));
    ENetPacket* packet = enet_packet_create (broadcast_stream.data(), broadcast_stream.size(), ENET_PACKET_FLAG_RELIABLE);
    enet_host_broadcast (host.load(), 0, packet);
    enet_host_flush (host.load());
}

//broadcast to all players in the server
void send_broadcast(const std::string& message)
{
    DataStream broadcast_stream(MSG_BUFFER_SIZE);
    broadcast_stream.write(Byte(0));
    broadcast_stream.write(Byte(255));
    broadcast_stream.write(message);
    ENetPacket* packet = enet_packet_create (broadcast_stream.data(), broadcast_stream.size(), ENET_PACKET_FLAG_RELIABLE);
    enet_host_broadcast (host.load(), 0, packet);
    enet_host_flush (host.load());
}

//called when a user disconnects
void user_disconnected(ENetEvent* event)
{
    //clear the client's enet peer, this is so we know they're logged in
    ClientState* client_state = client_manager.client_for_id(event->peer->data);
    if(client_state)
    {
        client_state->set_enet_peer(nullptr);

        std::string disconected_string = client_state->username();
        disconected_string.append(" disconected.");
        send_broadcast(disconected_string);
        std::cout << client_state->username() << " disconected." << std::endl;
    }
    delete (char*)event->peer->data;
    event->peer->data = nullptr;
}


//Function called when a new user connects to the server
//We check for a matching client state or create a new one for this user
void new_user(ENetEvent* event)
{
    DataStream event_stream((Byte*)event->packet->data, MSG_BUFFER_SIZE);
    event_stream.skip_forwards(1); // skip the first byte
    
    //get the client version
    Byte client_version;
    event_stream.read(client_version);

    if(client_version != CLIENT_VERSION_REQUIRED)
    {
        message_peer(event->peer, MESSAGE_TYPE_BAD_CLIENT_VERSION);
        std::cout << "Incorrect client tried to connect." << std::endl;
        enet_packet_destroy(event->packet);
        return;
    }

    //get the username out of the event
    const std::string event_username = event_stream.string(USERNAME_SIZE);
    // read out the password
    const std::string md5_password = event_stream.string(PASSWORD_SIZE); 

    //check if we already have a user of that name
    ClientState* client_ptr = client_manager.client_for_username(event_username);
    if(client_ptr)
    {
        if(client_ptr->password() == md5_password)
        {
            //we malloc the peer data when we register new clients
            //but if the server has restarted and we're using existing loaded client data
            //we can have existing users with non malloc'd peer data
            if(event->peer->data == nullptr)
            {
                event->peer->data = malloc(sizeof(char));
            }
            //write the client ID to their packet
            *(char*)event->peer->data = (char)client_ptr->ID();
            std::cout << "Returning user with username: " << client_ptr->username() << std::endl;
        }
        else
        {
            message_peer(event->peer, MESSAGE_TYPE_BAD_LOGIN);
            std::cout << "Attempted login to account: " << client_ptr->username() << std::endl;
            enet_packet_destroy(event->packet);
            return;
        }
    }
    else
    {
        //register a new client
        client_ptr = client_manager.register_new_client(event, event_username, md5_password);

        std::cout << "New user with username: " << client_ptr->username() << " : " << md5_password << std::endl;
    }

    //give the client data the event peer data so we can tell their login status
    client_ptr->set_enet_peer(event->peer);

    //prep a stream to send back to the new user with their unique ID
    DataStream stream(MSG_BUFFER_SIZE);
    stream.write(Byte(1));
    //write the client's ID to the stream so they can save it and use it later
    stream.write((char)client_ptr->ID());

    //tell the new user about their UniqueID
    ENetPacket* packet = enet_packet_create (stream.data(), stream.size(), ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send (client_ptr->enet_peer(), 0, packet);
    enet_host_flush (host.load());

    //send welcome string
    message_peer(client_ptr->enet_peer(), world_state.welcome_string());
    //send the user the information for where they currently are
    mud_look(event, std::vector<std::string>());

    enet_packet_destroy (event->packet);
}

//When a message is recieved from the player
//we evaluate ad a MUDAction and split the string into tokens
void message_recieved(ENetEvent* event)
{
    //verify the UserID first!
    DataStream stream((Byte*)event->packet->data, MSG_BUFFER_SIZE);
    stream.skip_forwards(1);
    ClientState* client_state = client_manager.client_for_id(event->peer->data);
    char id;
    stream.read(id); // read the id out of the stream
    if(client_state->ID() != id)
    {
        message_peer(event->peer, "Invalid Client ID, try logging out and in again!");
        return;
    }

    //Ha! I love C++
    std::vector<std::string> tokens;
    std::stringstream ss(stream.string(MSG_BUFFER_SIZE-2));
    std::string buffer;
    while(ss >> buffer)
    {
        tokens.push_back(buffer);
    }

    auto found_mud_action = std::find_if(mud_actions.begin(), mud_actions.end(), [tokens](const std::pair<const std::string&, MUDAction>& pair)
    {
        return tokens.front() == pair.first;
    });

    if(found_mud_action != mud_actions.end())
    {
        //call the matching mud action with the tokens we've seperated
        found_mud_action->second(event, tokens);
        command_history.push_back(std::make_pair(client_state->username(), tokens.front()));
    }
    else
    {
        message_peer(event->peer, "This is an unknown action, try using help!");
    }
    enet_packet_destroy (event->packet);
}

void message_peer(ENetPeer* peer, const std::string& str)
{
    //sends the string given to the peer passed
    //sends as a message from the server to the client, the client should just print this
    DataStream stream(MSG_BUFFER_SIZE);
    stream.write(Byte(0)); // Server to client message
    stream.write(Byte(255));
    stream.write(str);
    ENetPacket* packet = enet_packet_create (stream.data(), stream.size(), ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send (peer, 0, packet);
    enet_host_flush (host.load());
}

void message_peer(ENetPeer* peer, Byte byte)
{
    //sends a single byte packet to a peer, used for bad passwords, disconnects etc
    DataStream stream(1);
    stream.write(byte); // Server to client message
    ENetPacket* packet = enet_packet_create (stream.data(), stream.size(), ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send (peer, 0, packet);
    enet_host_flush (host.load());
}

//look around the players current position as stored in their client state
//sends the player strings describing their current location and directions relative to it
void mud_look(ENetEvent* event, std::vector<std::string> tokens)
{
    ClientState* client = client_manager.client_for_id(event->peer->data);
    const Location& client_loc = world_state.location(client->location_id());
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
    message_peer(event->peer, ss.str());
}

//allows players to talk to people at the same location to them
void mud_say(ENetEvent* event, std::vector<std::string> tokens)
{
    ClientState* client = client_manager.client_for_id(event->peer->data);
    if(tokens.size() < 2)
    {
        message_peer(event->peer, "This action needs parameters, try using help!");
    }
    else
    {
        std::stringstream ss;
        ss << client->username() << ":";
        //reconstruct from tokens, tbh this could be a lot better...
        for(int i = 1; i < tokens.size(); ++i)
        {
            ss << " " << tokens.at(i);
        }
        std::cout << ss.str() << std::endl;
        for(ClientState* state : client_manager.online_users())
        {
            if(state->enet_peer() && client->location_id() == state->location_id())
            {
                message_peer(state->enet_peer(), ss.str());
            }
        }
    }
}

//allows the players to move in n,e,s,w directions
//checks if in the map, if passable, and then applies the move their client state
void mud_go(ENetEvent* event, std::vector<std::string> tokens)
{
    ClientState* client = client_manager.client_for_id(event->peer->data);
    const Location& client_loc = world_state.location(client->location_id());
    if(tokens.size() < 2)
    {
        message_peer(event->peer, "This action needs parameters, try using help!");
    }
    else
    {
        const std::string invalid_direction = "You can't go in that direction";
        std::string response;
        std::string& param1 = tokens.at(1);
        if(param1 == "n" || param1 == "north")
        {
            if(client->location_id() < world_state.width())
            {
                response = invalid_direction;
            }
            else if (world_state.location(client->location_id()-world_state.width()).IsPassable(client) == false)
            {
                response = "That region is impassable.";
            }
            else
            {
                client->set_location(client->location_id()-world_state.width());
                response = "You travel north.";
            }
        }
        else if(param1 == "e" || param1 == "east")
        {
            if((client->location_id()+1) % world_state.width() == 0)
            {
                response = invalid_direction;
            }
            else if (world_state.location(client->location_id()+1).IsPassable(client) == false)
            {
                response = "That region is impassable.";
            }
            else
            {
                client->set_location(client->location_id()+1);
                response = "You travel east.";
            }
        }
        else if(param1 == "s" || param1 == "south")
        {
            if((client->location_id() / world_state.height()) >= world_state.height())
            {
                response = invalid_direction;
            }
            else if (world_state.location(client->location_id()+world_state.width()).IsPassable(client) == false)
            {
                response = "That region is impassable.";
            }
            else
            {
                client->set_location(client->location_id()+world_state.width());
                response = "You travel south.";
            }
        }
        else if(param1 == "w" || param1 == "west")
        {
            if(client->location_id() % world_state.width() == 0)
            {
                response = invalid_direction;
            }
            else if (world_state.location(client->location_id()-1).IsPassable(client) == false)
            {
                response = "That region is impassable.";
            }
            else
            {
                client->set_location(client->location_id()-1);
                response = "You travel west.";
            }
        }
        //todo: we may want to tell other players in the location that this player has left/joined
        message_peer(event->peer, response);
    }
}

//prints a bunch of commands, should probably just be in data somewhere but eh
void mud_help(ENetEvent* event, std::vector<std::string>)
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

    message_peer(event->peer, ss.str());
}

void mud_inv(ENetEvent* event, std::vector<std::string>)
{
    ClientState* client = client_manager.client_for_id(event->peer->data);
    if(client)
    {
        message_peer(event->peer, client->inventory().print_string(item_manager));
    }
}

void mud_quests(ENetEvent* event, std::vector<std::string> tokens)
{
    if(tokens.size() < 2)
    {
        message_peer(event->peer, "This action needs parameters, try using help!");
        return;
    }
    ClientState* client = client_manager.client_for_id(event->peer->data);
    if(client)
    {
        //get the client's location
        const Location& client_loc = world_state.location(client->location_id());
        std::stringstream ss;

        //if we're listing quests
        if(tokens.at(1) == "list")
        {
            //get the client's active quests
            ss << client->quest_status_string(quest_manager, false);
            ss << "---- Available Quests ----\n";
            //get the available quests at their location
            auto available = client_loc.available_quests(client);
            if(available.empty())
            {
                ss << "There are no quests at this location.\n";
            }
            else
            {
                //create the strings for each available quest
                for(int quest_id : available)
                {
                    Quest* quest = quest_manager.quest_for_id(quest_id);
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
                message_peer(event->peer, "This action needs parameters, try using help!");
                return;
            }

            //get the 3rd argument as an int
            std::istringstream int_ss(tokens.at(2));
            if(int_ss.fail())
            {
                message_peer(event->peer, "That wasn't an ID! Try giving numbers");
                return;
            }
            int quest_id;
            int_ss >> quest_id;

            //accept a quest at this location
            if(tokens.at(1) == "accept")
            {
                //check if it's available
                if(client_loc.is_quest_available(client, quest_id))
                {
                    //accept if it is
                    client->accept_quest(quest_id);
                    //construct strings for acceptance
                    ss << "Quest accepted!\n";
                    Quest* quest = quest_manager.quest_for_id(quest_id);
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
                if(client->has_active_quest(quest_id))
                {
                    client->abandon_quest(quest_id);
                    ss << "Quest abandoned.\n";
                }
                else
                {
                    ss << "You're not currently on that quest.\n";
                }
            }
            else if(tokens.at(1) == "complete")
            {
                Quest* quest = quest_manager.quest_for_id(quest_id);
                if(quest && client->has_active_quest(*quest))
                {
                    switch(quest_manager.completion_status(*quest, client, world_state.location(client->location_id())))
                    {
                        case QUEST_COMPLETION_STATUS::COMPLETE:
                        {
                            client->complete_quest(*quest);
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
        message_peer(event->peer, ss.str());
    }
}

void mud_pickup(ENetEvent* event, std::vector<std::string> tokens)
{
    if(tokens.size() < 2)
    {
        message_peer(event->peer, "This action needs parameters, try using help!");
        return;
    }
    
    //get the 2nd argument as an int
    std::istringstream int_ss(tokens.at(1));
    if(int_ss.fail())
    {
        message_peer(event->peer, "That wasn't an ID! Try giving numbers");
        return;
    }
    int item_id;
    int_ss >> item_id;
    std::stringstream ss;

    ClientState* client = client_manager.client_for_id(event->peer->data);
    if(client)
    {
        for(int available_item_id : world_state.location(client->location_id()).m_available_items)
        {
            if(available_item_id == item_id)
            {
                client->inventory().gain_item(item_id);
                message_peer(event->peer, "You picked up the item.\n");
                return;
            }
        }
        message_peer(event->peer, "That item isn't available here.\n");
    }
}