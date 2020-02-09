all:
	g++ *.cpp /usr/local/lib/libenet.a -o output -g -std=c++17 -Wno-int-to-void-pointer-cast -Wno-format -lpthread
OSXstandalone:
	g++ *.cpp -o output -std=c++17 -Wno-switch /usr/local/lib/libenet.a -mmacosx-version-min=10.9