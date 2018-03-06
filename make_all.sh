#!/bin/bash
./get_enet.sh
cd server
make
cd ../client
make
echo "Make All complete :)"
echo "You can run the output binaries in the client and server folders."