#include "Utils.h"
#include "NetServer.h"
#include "NetCommon.h"

const int SERVER_PORT = 5000;

void processSelfId(const std::string& msg)
{
    auto splits = Utils::split(msg,":");

    Utils::log("user is",splits[1]);
}

int main()
{
    NetServer server;

    server.startServer(SERVER_PORT);


    int fd; 
    if(!server.acceptConnection(fd))
    {
        Utils::log("Connection Failed!");
    }

    Utils::log("connected with FD",fd);

    std::string msg;
    if(!NetCommon::recvMsg(fd,msg))
    {
        Utils::log("recv Failed!");
    }

    processSelfId(msg);


    Utils::log("Received",msg);

    return 0;
}
