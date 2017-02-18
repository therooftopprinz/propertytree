echo compiling
g++ -std=c++14 -ggdb3 -I ../clientlib/ -c testing.cpp -o testing.o
echo linking
g++ -std=c++14 testing.o ../clientlib/client.a ../clientlib/common.a -lpthread -o testing