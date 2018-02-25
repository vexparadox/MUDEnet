#include "main.hpp"
int main(int argc, char const *argv[])
{
    if (enet_initialize () != 0)
    {
        fprintf (stderr, "An error occurred while initializing ENet.\n");
        return 1;
    }
    atexit (enet_deinitialize);

    if(argc < 3)
    {
    	fprintf(stderr, "No address or port was supplied\n");
    	return 1;
    }
	clientCount = 0;
    enet_address_set_host(&serverAddress, argv[1]);
    serverAddress.port = atoi(argv[2]);

    host.store(enet_host_create (&serverAddress, 32, 2, 0, 0));
    if (!host.load()) 
    { 
        printf("%s\n", "An error occurred while trying to create the server host."); 
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

    printf("Server was started on %s:%s, now listening for clients.\n", argv[1], argv[2]);
    std::thread inputThread(&takeInput);

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
		        printf ("A new client connected from %x:%u.\n", 
		                event.peer -> address.host,
		                event.peer -> address.port);

                //make space for the new users ID
				event.peer->data = malloc(sizeof(char));
                //write the new client Count
				*(char*)event.peer->data = clientCount;
				clientCount++; // increment the id

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
                userDisconnected(&event);
                break;
            case ENET_EVENT_TYPE_NONE:
                break;
            }
        }

    }

    enet_host_destroy(host);
    inputThread.join();
	return 0;
}

void takeInput()
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
            // controller->takeInput(buffer);
            if(strcmp(buffer, "exit") == 0)
            {
        		run.store(false);
        	}else
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

void sendBroadcast()
{
    ENetPacket* packet = enet_packet_create (broadcast_stream.data(), broadcast_stream.size(), ENET_PACKET_FLAG_RELIABLE);
    enet_host_broadcast (host.load(), 0, packet);
    enet_host_flush (host.load());
}

void userDisconnected(ENetEvent* event)
{
    const char user_id = *(char*)event->peer->data;
    broadcast_stream.clear_data();
    broadcast_stream.write(Byte(2)); // user disconnected
    broadcast_stream.write(user_id, 1); // copy user ID, first byte of peer data
    sendBroadcast(); // send the broadcast
    event->peer->data = NULL;

    std::cout << ClientStateForID(user_id)->Username() << " disconected." << std::endl;

}

void newUser(ENetEvent* event)
{
    broadcast_stream.clear_data();
    broadcast_stream.write(Byte(1));

    //check if we already have a username with this ID
    const char* event_username = (char*)event->packet->data+2;
    auto found_user = std::find_if(client_states.begin(), client_states.end(), [event_username](const ClientState& state)
    {
        return strcmp(event_username, state.Username().c_str()) == 0;
    });

    if(found_user != client_states.end())
    {
        //write over the peer's id so it's correct for further messages
        *(char*)event->peer->data = (char)found_user->ID();
        broadcast_stream.write((char)found_user->ID()); // write the users ID
        broadcast_stream.write(found_user->Username()); // copy the old username
        std::cout << "Returning user with id/name: " << found_user->ID() << "/" << found_user->Username() << std::endl;
    }
    else
    {
        ClientState new_state(*(char*)event->peer->data, (char*)event->packet->data+2);
        client_states.push_back(new_state);

        broadcast_stream.write((char)new_state.ID()); // write the users ID
        broadcast_stream.write(new_state.Username()); // copy the new username for broadcast
        std::cout << "New user with id/name: " << new_state.ID() << "/" << new_state.Username() << std::endl;
    }
    sendBroadcast();

    //tell the new user about old ones
    DataStream stream(1024);
    for(const auto& client : client_states)
    {
        stream.clear_data();
        stream.write(Byte(1)); // new username
        stream.write(client.ID());
        stream.write(client.Username());
        ENetPacket* packet = enet_packet_create (stream.data(), stream.size(), ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send (event->peer, 0, packet);
        enet_host_flush (host.load());
    }    

    //send the user the information for where they currently are
    mud_look(event, std::vector<std::string>());

    enet_packet_destroy (event->packet);
}

void messageRecieved(ENetEvent* event)
{

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
        found_mud_action->second(event, tokens);
    }
    else
    {
        respond_to_sender(event, "This is an unknown action, try using help!");
    }
    enet_packet_destroy (event->packet);
}

void respond_to_sender(ENetEvent* event, const std::string& str)
{
    DataStream stream(1024);
    stream.clear_data();
    stream.write(Byte(0)); // new username
    stream.write(Byte(255));
    stream.write(str);
    ENetPacket* packet = enet_packet_create (stream.data(), stream.size(), ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send (event->peer, 0, packet);
    enet_host_flush (host.load());
}

void mud_look(ENetEvent* event, std::vector<std::string> tokens)
{
    ClientState* client = ClientStateForID(*(char*)event->peer->data);
    const Location& client_loc = world_state.Locations().at(client->LocationID());
    std::string location;
    location.append("\n");
    location.append("----" + client_loc.m_title + "----");
    location.append("\n");
    location.append(client_loc.m_description);
    location.append("\n");
    location.append("-----------------------\n");
    if(tokens.size() < 2)
    {
        location.append(client_loc.m_here);
    }
    else
    {
        std::string& param1 = tokens.at(1);
        if(param1 == "n" || param1 == "north")
        {
            location.append(client_loc.m_north);
        }
        else if(param1 == "e" || param1 == "east")
        {
            location.append(client_loc.m_east);
        }
        else if(param1 == "s" || param1 == "south")
        {
            location.append(client_loc.m_south);
        }
        else if(param1 == "w" || param1 == "west")
        {
            location.append(client_loc.m_west);
        }
        else
        {
            location.append(client_loc.m_here);
        }
    }
    respond_to_sender(event, location);
}

void mud_say(ENetEvent* event, std::vector<std::string> tokens)
{
    if(tokens.size() < 2)
    {
        respond_to_sender(event, "This action needs parameters, try using help!");
    }
    else
    {
        std::cout << "say" << std::endl;
    }
}

void mud_go(ENetEvent* event, std::vector<std::string> tokens)
{
    ClientState* client = ClientStateForID(*(char*)event->peer->data);
    const Location& client_loc = world_state.Locations().at(client->LocationID());
    if(tokens.size() < 2)
    {
        respond_to_sender(event, "This action needs parameters, try using help!");
    }
    else
    {
        std::string response;
        std::string& param1 = tokens.at(1);
        if(param1 == "n" || param1 == "north")
        {
            if(client->LocationID() < world_state.m_world_width)
            {
                response = "You can't go in that direction.";
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
                response = "You can't go in that direction.";
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
                response = "You can't go in that direction.";
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
                response = "You can't go in that direction.";
            }
            else
            {
                client->SetLocation(client->LocationID()-1);
                response = "You travel west.";
            }
        }
        respond_to_sender(event, response);
    }
}

void mud_help(ENetEvent* event, std::vector<std::string>)
{
    std::string help_str = "\n";
    help_str.append("You can use the following commands:");
    help_str.append("\n");
    help_str.append("go <direction>");
    help_str.append("\n");
    help_str.append("look (direction)");
    help_str.append("\n");
    help_str.append("say <message>");
    help_str.append("\n");
    help_str.append("exit");

    respond_to_sender(event, help_str);
}
ClientState* ClientStateForID(char id)
{
    auto found_user = std::find_if(client_states.begin(), client_states.end(), [id](const ClientState& state)
    {
        return id == (char)state.ID();
    });
    if(found_user != client_states.end())
    {
        return &(*found_user);
    }
    return nullptr;
}
