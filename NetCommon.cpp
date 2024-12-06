#include "NetCommon.h"

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


#include "Utils.h"

const int BUFFER_SIZE = 1024;


bool NetCommon::recvMsg(const int& fd, std::string& out)
{
    Utils::log("Try rx");
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    ssize_t bytes_received = recv(fd, buffer, BUFFER_SIZE, 0);
    Utils::log("got",bytes_received,"bytes");

    out = buffer;
    return true;
}

bool NetCommon::sendMsg(const int& fd, const std::string& in)
{
    send(fd, in.c_str(), in.size(), 0);

    return true;
}

bool NetCommon::connectTo(int& fd, const std::string& ip, const int port)
{
    // Create an SCTP socket
    fd = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);
    if (fd < 0) {
        perror("Socket creation failed");
        return false;
    }


    sockaddr_in server_addr{};
    // Configure the server address
    server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr);
    server_addr.sin_port = htons(port);

    // Connect to the server
    if (connect(fd, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(fd);
        return false;
    }

    return true;
}
