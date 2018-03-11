# MUDEnet system
This is a MUD built using Enet in C++!


## Todos!

* Create a build script for Windows

* Write out world state upon exit? (currently nothing in the world state is actually modified)

* Add in pickupable/awardable items

* Add in quests users can pickup

* Add movement that requires items (machette for forest eg)

* Improve the DataStream to handle more types

## Requirements & Setup
Run the "make_all.sh" script to install enet and build the binaries :)

* Start using `./output ip port`

* Make sure the server has started before the client. 

NB - If the server/client fails to start it could be caused by you using an ip/port that's not available.


## Packet Config
Each packet is 1024 bytes, BN means byte N of 1024 in a packet.

B1 is the ID of the packets action, this can be used in a switch or array of functions.
### Server to Client packets:
|      Name      | B1 |   B2  | B3 to B1024     |                                         Notes                                        |
|:--------------:|:--:|:-----:|-----------------|:------------------------------------------------------------------------------------:|
|     Message    |  0 | 255 | Message Content | The client prints this message.        |
| New user |  1 | 0-255 | NULL        | B2 is userID, this is the clients secret ID they use to communicate with the server. |
| Server Killed |  2 | NULL | NULL        | Server has terminated, shut down the client |
| Bad Password |  3 | NULL | NULL        | Bad password given, shut down the client |

### Client to Server packets:
Note that B2 is used for UserIDs, the Client doesn't need to know about its own ID. This will be filled by the server. Leave B2 empty for clarity, it's ignored by the Server anyway.

|   Name  | B1 |  B2  | B3 to B1024      |                         Notes                        |
|:-------:|:--:|:----:|-----------------|:----------------------------------------------------:|
| Message |  0 | NULL | Message Content | Used to send commands to the server |
| Username |  1 | NULL | Username | B3-B510 used as a login and retrieve the existing/new client state. A unique ID will be returned to this user only!|