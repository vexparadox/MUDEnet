#!/bin/bash
mkdir temp
cd temp
echo "This script will download and install enet, it will require sudo permissions to copy"
curl -Ls http://enet.bespin.org/download/enet-1.3.13.tar.gz -o enet-1.3.13.tar.gz
tar -xzf enet-1.3.13.tar.gz
cd enet-1.3.13
./configure
make && sudo make install
cd ../../
sudo rm -rf temp
echo "Install complete!"