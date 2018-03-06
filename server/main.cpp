#include "main.hpp"
int main(int argc, char const *argv[])
{
    if (enet_initialize () != 0)
    {
        std::cout << "An error occurred while initializing ENet." << std::endl;
        return 1;
    }
    atexit (enet_deinitialize);

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
    run.store(true);


    //make the pairs of mud functions
    mud_actions.push_back(std::make_pair("look", mud_look));
    mud_actions.push_back(std::make_pair("say", mud_say));
    mud_actions.push_back(std::make_pair("go", mud_go));
    mud_actions.push_back(std::make_pair("help", mud_help));

    world_state.load("map.json");
    world_state.parse();

    std::cout << "Server was started on " << argv[1] << ":" << argv[2] << std::endl;

    //start input thread
    std::thread inputThread(&take_input);

    while(run.load()){
        ENetEvent event;
        //wait upto half a second for an event
        while (enet_host_service (host.load(), &event, 0) > 0)
        {
            switch (event.type)
            {
            //When a player connects
            case ENET_EVENT_TYPE_CONNECT:
            {
                std::cout << std::endl << "A new player connected from " << event.peer->address.host << ":" << event.peer->address.port << std::endl;
                broadcast_stream.clear_data();
                broadcast_stream.write(Byte(0));
                broadcast_stream.write(Byte(255));
                broadcast_stream.write(world_state.m_welcome_string);
                //create a packet and send
        	    ENetPacket* packet = enet_packet_create (broadcast_stream.data(), broadcast_stream.size(), ENET_PACKET_FLAG_RELIABLE);
				enet_peer_send (event.peer, 0, packet);
			    enet_host_flush (host.load());
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

    }

    enet_host_destroy(host);
    inputThread.join();

    for(ClientState* state : client_states)
    {
        delete state;
        state = nullptr;
    }

	return 0;
}

void take_input()
{
    char buffer[510];
    while (run.load()){
        DataStream stream(1024);
        memset(buffer, 0, 510);
        fgets(buffer, 510, stdin);
        //get rid of that pesky \n
        char* temp = buffer+strlen(buffer)-1;
        *temp = '\0';
        stream.write(Byte(0)); // this is a 0 action!
        stream.write(Byte(255)); // set the ID to 255, this is reserved for the server
        if(strcmp(buffer, "") != 0)
        {
            if(strcmp(buffer, "exit") == 0)
            {
        		run.store(false);
        	}
            else if(strcmp(buffer, "list") == 0)
            {
                print_users();
            }
            else
            {
                stream.write(buffer, 510);
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

//broadcast to all players in the server
void send_broadcast()
{
    ENetPacket* packet = enet_packet_create (broadcast_stream.data(), broadcast_stream.size(), ENET_PACKET_FLAG_RELIABLE);
    enet_host_broadcast (host.load(), 0, packet);
    enet_host_flush (host.load());
}

//called when a user disconnects
void user_disconnected(ENetEvent* event)
{
    //clear the client's enet peer, this is so we know they're logged in
    ClientState* client_state = (ClientState*)event->peer->data;
    client_state->SetENetPeer(nullptr);

    broadcast_stream.clear_data();
    broadcast_stream.write(Byte(0));
    broadcast_stream.write(Byte(255));
    std::string disconected_string = client_state->Username();
    disconected_string.append(" disconected.");
    broadcast_stream.write(disconected_string); // copy user ID, first byte of peer data
    send_broadcast(); // send the broadcast
    event->peer->data = nullptr;
    std::cout << client_state->Username() << " disconected." << std::endl;

}

void print_users()
{
    std::cout << "Online Users: " << std::endl;
    for(ClientState* state : client_states)
    {
        if(state && state->Peer())
        {
            std::cout << state->Username() << std::endl;
        }
    }
}

//Function called when a new user connects to the server
//We check for a matching client state or create a new one for this user
void new_user(ENetEvent* event)
{
    //prep a stream to send back to the new user with their unique ID
    DataStream stream(1024);
    stream.write(Byte(1));

    //check if we already have a username with this ID
    const char* event_username = (char*)event->packet->data+2;
    auto found_user = std::find_if(client_states.begin(), client_states.end(), [event_username](const ClientState* state)
    {
        return strcmp(event_username, state->Username().c_str()) == 0;
    });

    if(found_user != client_states.end())
    {
        //write over the peer's id so it's correct for further messages
        event->peer->data = (void*)(*found_user);
        (*found_user)->SetENetPeer(event->peer);
        stream.write((char)(*found_user)->ID()); // write the users ID
        std::cout << "Returning user with id/name: " << (*found_user)->ID() << "/" << (*found_user)->Username() << std::endl;
    }
    else
    {
        ClientState* new_state = new ClientState(client_states.size(), (char*)event->packet->data+2);
        new_state->SetENetPeer(event->peer);
        event->peer->data = (void*)new_state;
        stream.write(new_state->ID());
        std::cout << "New user with id/name: " << new_state->ID() << "/" << new_state->Username() << std::endl;
        client_states.emplace_back(new_state);
    }

    //tell the new user about their UniqueID
    ENetPacket* packet = enet_packet_create (stream.data(), stream.size(), ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send (event->peer, 0, packet);
    enet_host_flush (host.load());

    //send the user the information for where they currently are
    mud_look(event, std::vector<std::string>());

    enet_packet_destroy (event->packet);
}

//When a message is recieved from the player
//we evaluate ad a MUDAction and split the string into tokens
void message_recieved(ENetEvent* event)
{
    //verify the UserID first!
    DataStream stream((Byte*)event->packet->data, 2);
    stream.skip_forwards(1);
    const char id = *stream.read<char>(); // read the ID given by the user to the server
    if(((ClientState*)event->peer->data)->ID() != id)
    {
        message_peer(event->peer, "Invalid Client ID, try logging out and in again!");
        return;
    }

    //Ha! I love C++
    std::vector<std::string> tokens;
    std::string input((char*)event->packet->data+2);
    std::stringstream ss(input);
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
    DataStream stream(1024);
    stream.clear_data();
    stream.write(Byte(0)); // Server to client message
    stream.write(Byte(255));
    stream.write(str);
    ENetPacket* packet = enet_packet_create (stream.data(), stream.size(), ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send (peer, 0, packet);
    enet_host_flush (host.load());
}

//look around the players current position as stored in their client state
//sends the player strings describing their current location and directions relative to it
void mud_look(ENetEvent* event, std::vector<std::string> tokens)
{
    ClientState* client = (ClientState*)event->peer->data;
    const Location& client_loc = world_state.Locations().at(client->LocationID());
    std::stringstream ss;
    ss << "\n ----" << client_loc.m_title << "----" << "\n";
    if(tokens.size() < 2)
    {
        ss << client_loc.m_description << "\n ----------------------- \n";
        ss << client_loc.m_here;
    }
    else
    {
        std::string& param1 = tokens.at(1);
        if(param1 == "n" || param1 == "north")
        {
            ss << client_loc.m_north;
        }
        else if(param1 == "e" || param1 == "east")
        {
            ss << client_loc.m_east;
        }
        else if(param1 == "s" || param1 == "south")
        {
            ss << client_loc.m_south;
        }
        else if(param1 == "w" || param1 == "west")
        {
            ss << client_loc.m_west;
        }
        else
        {
            ss << client_loc.m_here;
        }
    }
    message_peer(event->peer, ss.str());
}

//allows players to talk to people at the same location to them
void mud_say(ENetEvent* event, std::vector<std::string> tokens)
{
    ClientState* client = (ClientState*)event->peer->data;
    if(tokens.size() < 2)
    {
        message_peer(event->peer, "This action needs parameters, try using help!");
    }
    else
    {
        std::stringstream ss;
        ss << client->Username() << ":";
        //reconstruct from tokens, tbh this could be a lot better...
        for(int i = 1; i < tokens.size(); ++i)
        {
            ss << " " << tokens.at(i);
        }

        for(ClientState* state : client_states)
        {
            if(state->Peer() && client->LocationID() == state->LocationID())
            {
                message_peer(state->Peer(), ss.str());
            }
        }
    }
}

//allows the players to move in n,e,s,w directions
//checks if in the map, if passable, and then applies the move their client state
void mud_go(ENetEvent* event, std::vector<std::string> tokens)
{
    ClientState* client = (ClientState*)event->peer->data;
    const Location& client_loc = world_state.Locations().at(client->LocationID());
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
            if(client->LocationID() < world_state.m_world_width)
            {
                response = invalid_direction;
            }
            else if (world_state.Locations().at(client->LocationID()-world_state.m_world_width).IsPassable(client) == false)
            {
                response = "That region is impassable.";
            }
            else
            {
                client->SetLocation(client->LocationID()-world_state.m_world_width);
                response = "You travel north.";
            }
        }
        else if(param1 == "e" || param1 == "east")
        {
            if((client->LocationID()+1) % world_state.m_world_width == 0)
            {
                response = invalid_direction;
            }
            else if (world_state.Locations().at(client->LocationID()+1).IsPassable(client) == false)
            {
                response = "That region is impassable.";
            }
            else
            {
                client->SetLocation(client->LocationID()+1);
                response = "You travel east.";
            }
        }
        else if(param1 == "s" || param1 == "south")
        {
            if((client->LocationID() / world_state.m_world_height) >= world_state.m_world_height)
            {
                response = invalid_direction;
            }
            else if (world_state.Locations().at(client->LocationID()+world_state.m_world_width).IsPassable(client) == false)
            {
                response = "That region is impassable.";
            }
            else
            {
                client->SetLocation(client->LocationID()+world_state.m_world_width);
                response = "You travel south.";
            }
        }
        else if(param1 == "w" || param1 == "west")
        {
            if(client->LocationID() % world_state.m_world_width == 0)
            {
                response = invalid_direction;
            }
            else if (world_state.Locations().at(client->LocationID()-1).IsPassable(client) == false)
            {
                response = "That region is impassable.";
            }
            else
            {
                client->SetLocation(client->LocationID()-1);
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
    ss << "go <direction>" << "\n";
    ss << "look (direction)" << "\n";
    ss << "say <message>" << "\n";
    ss << "exit";

    message_peer(event->peer, ss.str());
}