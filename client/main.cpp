#include "main.hpp"
int main(int argc, char const *argv[])
{
	running.store(true);
    if (enet_initialize () != 0)
    {
        fprintf (stderr, "An error occurred while initializing ENet.\n");
        return EXIT_FAILURE;
    }
    atexit (enet_deinitialize);
    if(argc < 3){
    	fprintf(stderr, "No address or port was supplied.\n");
    	return EXIT_FAILURE;
    }
    //create a client
    client.store(enet_host_create (NULL, 1, 2, 57600 / 8, 57600 / 8));
    //if the client failed to create
    if (client.load() == NULL){
	    fprintf (stderr, "An error occurred while trying to create an ENet client host.\n");
	    exit (EXIT_FAILURE);
	}
	//setup the clientAddress
	enet_address_set_host (&clientAddress, argv[1]);
    clientAddress.port = atoi(argv[2]);

    ENetEvent event;
    peer.store(enet_host_connect (client.load(), &clientAddress, 2, 0));
    enet_host_flush (client.load());
    if (!peer.load()){
    	printf("%s\n", "No available peers for initiating an ENet connection.");
    }

    printf("%s\n", "Attempting to connect...");

    if (enet_host_service (client.load(), &event, 2000) > 0 &&
        event.type == ENET_EVENT_TYPE_CONNECT)
    {
    	printf("Connection to %s:%s succeeded on channel 0.\n", argv[1], argv[2]);
        connected = true;
        event.peer -> data = (void*)"Server";
        server.store(event.peer);
    }else{
        enet_peer_reset (peer.load());
        connected = false;
        printf("%s\n", "Failed to connect.");
        running.store(false);
    }

    names = (char**)malloc(sizeof(char)*255*sizeof(char*));
    for(int i = 0; i < 255; i++){
        names[i] = (char*)malloc(sizeof(char)*510);
    }
    names[255] = "Server";
    std::thread inputThread(&takeInput);
    while(running.load() && connected){
        ENetEvent event;
        //wait upto half a second for an event
        while (enet_host_service (client.load(), &event, 500) > 0)
        {
            switch (event.type)
            {
            case ENET_EVENT_TYPE_RECEIVE:{
                //if it's a message
                if(event.packet->data[0] == 0){
    		        printf ("%x: %s \n",
    		                names[event.packet -> data[1]],
    		                event.packet -> data+2
    		                );
                }else if (event.packet->data[0] == 1){
                    //else if it's a new connection
                    memcpy(names[event.packet->data[1]], event.packet->data+2, 510); 
                    printf("New user with the name: %s\n", names[event.packet->data[1]]);
                    enet_packet_destroy (event.packet);
                }
            }break;
            case ENET_EVENT_TYPE_DISCONNECT:
                printf ("%s disconnected.\n", event.peer->data);
                event.peer -> data = NULL;
                running.store(false);
                break;

            case ENET_EVENT_TYPE_NONE:
                break;
            }
        }

    }
    inputThread.join();
    disconnect();
    return 0;
}

void takeInput(){
    char buffer[512];
    while (running.load()){
        fgets(buffer+2, 510, stdin);
        //get rid of that pesky \n
        char* temp = buffer+strlen(buffer)-1;
        *temp = '\0';
        buffer[0] = 0; // tell the server it's a message
        buffer[1] = 0; // the second byte is ignored by the server, it's filled with the userID when it's resent out
        if(strcmp(buffer, "") != 0){
            // controller->takeInput(buffer);
            if(strcmp(buffer, "exit") == 0){
        		running.store(false);
        	}else{
        	    ENetPacket* packet = enet_packet_create (buffer, 512, ENET_PACKET_FLAG_RELIABLE);
			    enet_peer_send (server.load(), 0, packet);         
			    enet_host_flush (client.load());
			    printf("\033[1A"); //go up one line
			    printf("\033[K"); //delete to the end of the line	
        	}
        }
    }
}

void disconnect(){
    ENetEvent event;
    enet_peer_disconnect (peer.load(), 0);
    /* Allow up to 3 seconds for the disconnect to succeed
    * and drop any packets received packets.
    */
    while (enet_host_service (client.load(), & event, 3000) > 0)
    {
        switch (event.type)
        {
        case ENET_EVENT_TYPE_RECEIVE:
            enet_packet_destroy (event.packet);
        break;
        case ENET_EVENT_TYPE_DISCONNECT:
            printf("%s\n", "Disconnection succeeded");
        return;
        }
    }
    //reset if we reach here, this is a forcefull disconnect
    enet_peer_reset (peer.load());
}