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

#include "NetCommon.h"
#include "Utils.h"

const std::string SERVER_IP = "127.0.0.1";
const int SERVER_PORT = 5000;
const int BUFFER_SIZE = 1024;

int main() {
    int client_fd;

    // Create an SCTP socket
    if(!NetCommon::connectTo(client_fd,SERVER_IP,SERVER_PORT))
    {
        Utils::log("Could not connect");
        return 1;
    }

    std::string message = "id:bob";



    NetCommon::sendMsg(client_fd,message);

    std::string msg;
    if(!NetCommon::recvMsg(client_fd,msg))
    {
        Utils::log("recv Failed!");
    }

    close(client_fd);
    return 0;
}
