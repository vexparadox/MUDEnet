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

    if(model.Initialise() == false)
    {
        return 1;
    }
    
    //deinit enet
    atexit (enet_deinitialize);

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
            model.Save();
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
    // todo: please fix this mad maths, maybe pre-alloc the buffer?
    const int message_content_size = MSG_BUFFER_SIZE-2-sizeof(size_t);
    char buffer[message_content_size];
    while (run.load()){
        DataStream stream(MSG_BUFFER_SIZE);
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
                model.Save();
        		run.store(false);
        	}
            else if(strcmp(buffer, "list") == 0)
            {
                model.GetClientManager().print_users();
            }
            else if(strcmp(buffer, "save") == 0)
            {
                model.Save();
            }
            else
            {
                stream.write_buffer(buffer, std::strlen(buffer));
        	    ENetPacket* packet = enet_packet_create (stream.data(), stream.size(), ENET_PACKET_FLAG_RELIABLE);
			    enet_host_broadcast (host.load(), 0, packet);         
			    enet_host_flush (host.load());	
			    printf("\033[1A"); //go up one line
			    printf("\033[K"); //delete to the end of the line
			    printf("\rServer: %s\n", buffer); // use \r to get back to the start and print
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
    ClientState* client_state = model.GetClientManager().client_for_id(event->peer->data);
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
    const std::string event_username = event_stream.string();
    // read out the password
    const std::string md5_password = event_stream.string(); 

    //check if we already have a user of that name
    ClientState* client_ptr = model.GetClientManager().client_for_username(event_username);
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
        client_ptr = model.GetClientManager().register_new_client(event, event_username, md5_password);

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
    message_peer(client_ptr->enet_peer(), model.GetWorldState().welcome_string());
    //send the user the information for where they currently are
    //mud_look(*client_ptr, std::vector<std::string>());
}

//When a message is recieved from the player
//we evaluate ad a MUDAction and split the string into tokens
void message_recieved(ENetEvent* event)
{
    //verify the UserID first!
    DataStream stream((Byte*)event->packet->data, MSG_BUFFER_SIZE);
    stream.skip_forwards(1);
    ClientState* client_state = model.GetClientManager().client_for_id(event->peer->data);
    char id;
    stream.read(id); // read the id out of the stream
    if(client_state == nullptr || client_state->ID() != id)
    {
        message_peer(event->peer, "Invalid Client ID, try logging out and in again!");
        return;
    }

    //Ha! I love C++
    std::vector<std::string> tokens;
    std::stringstream ss(stream.string());
    std::string buffer;
    while(ss >> buffer)
    {
        tokens.push_back(buffer);
    }

    const std::string& response = model.ProcessAction(*client_state, tokens);
    if(response.empty() == false)
    {
        message_peer(*client_state, response);
    }

    enet_packet_destroy (event->packet);
}

void message_peer(const ClientState& client_state, const std::string& str)
{
    if(client_state.enet_peer())    
    {
        message_peer(client_state.enet_peer(), str);
    }
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