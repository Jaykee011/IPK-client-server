SERVER = server
CLIENT = client

SERVER_S = server.cpp
CLIENT_S = client.cpp

CC=g++
CFLAGS=-std=c++11

all: client server

client: client.h client.cpp messaging.h
	$(CC) $(CFLAGS) $(CLIENT_S) -o ftrest

server: server.h server.cpp messaging.h
	$(CC) $(CFLAGS) $(SERVER_S) -o ftrestd

clean:
	rm ftrest
	rm ftrestd