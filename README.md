# MUDEnet system
This is a MUD built using Enet in C++!


## Todos!

* Create a build script for Windows

* Write out the client states upon exit

* Write out world state upon exit? (currently nothing in the world state is actually modified)

* Add in pickupable/awardable items

* Add movement that requires items (machette for forest eg)

* Add in MUD say command

* Improve the DataStream and use in the neglected Client code for clarity

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
|     Message    |  0 | 0-255 | Message Content | B2 is userID followed by message.        |
| New user |  1 | 0-255 | Username        | B2 is userID, following bytes by username given. |
| User disconnected |  2 | 0-255 | NULL        | B2 has disconnected |

### Client to Server packets:
Note that B2 is used for UserIDs, the Client doesn't need to know about its own ID. This will be filled by the server. Leave B2 empty for clarity, it's ignored by the Server anyway.

|   Name  | B1 |  B2  | B3 to B512      |                         Notes                        |
|:-------:|:--:|:----:|-----------------|:----------------------------------------------------:|
| Message |  0 | NULL | Message Content | B2 is filled by the Server when broadcast to clients |
| Username |  1 | NULL | Username | B3-B510 is saved and broadcast to other clients (only sent once!)|