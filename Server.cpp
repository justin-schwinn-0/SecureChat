#include "Utils.h"
#include "NetServer.h"
#include "NetCommon.h"

#include "ServerData.h"

const int SERVER_PORT = 5000;

void processSelfId(int& fd,const std::string& msg)
{
    auto splits = Utils::split(msg,":");

    Utils::log("user is",splits[1],NetCommon::getIp(fd));

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

    ServerData data;


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

    processSelfId(fd,msg);

    sendList(fd);

    if(!NetCommon::recvMsg(fd,msg))
    {
        Utils::log("recv Failed!");
    }

    Utils::log("Received",msg);

    return 0;
}
