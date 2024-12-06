#include "Utils.h"
#include "NetServer.h"
#include "NetCommon.h"

const int SERVER_PORT = 5000;

void processSelfId(const std::string& msg)
{
    auto splits = Utils::split(msg,":");

    Utils::log("user is",splits[1]);


}

void sendList(int& fd)
{
    std::vector<std::string> list = {"alice","tom"};

    std::string msg = "list:";
    for(auto s: list)
    {
        msg += s+":";
    }

    NetCommon::sendMsg(fd,msg);
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

    sendList(fd);


    Utils::log("Received",msg);

    return 0;
}
