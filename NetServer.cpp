#include "NetServer.h"
#include "Utils.h"

#include <iostream>
#include <climits>
#include <thread>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <sys/socket.h>
#include <sys/types.h>
#include <poll.h>
#include <sys/select.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <arpa/inet.h>


NetServer::NetServer():
    mListenFd(-1)
{
}

NetServer::~NetServer()
{
}

bool NetServer::startServer(int port)
{
    sockaddr_in server_addr{};

    // Create an SCTP socket
    mListenFd = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);
    
    if (mListenFd < 0) 
    {
        perror("Socket creation failed");
        return false;
    }

    // Configure the server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // Bind the socket to the address
    if (bind(mListenFd, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) 
    {
        perror("Bind failed");
        close(mListenFd);
        return false;
    }

    // Start listening for connections
    if (listen(mListenFd, 5) < 0) {
        perror("Listen failed");
        close(mListenFd);
        return false;
    }

    Utils::log("Server listening on port", port,"...\n");
    return true;
}

bool NetServer::acceptConnection(int& fd)
{
    if(mListenFd == -1)
    {
        Utils::log("Socket is bad!",mListenFd);
        return false;
    }

    sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);
    fd = accept(mListenFd, (sockaddr*)&client_addr, &client_len);
    if (fd < 0) {
        perror("Accept failed");
        return false;
    }

    return true;

}
