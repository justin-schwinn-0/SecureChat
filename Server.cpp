#include "Utils.h"
#include "NetServer.h"
#include "NetCommon.h"

#include "ServerData.h"

#include <thread>

const int SERVER_PORT = 5000;

void processSelfId(int& fd,const std::string& msg,ServerData& sd)
{
    auto splits = Utils::split(msg,":");

    std::string ip = NetCommon::getIp(fd);

    Utils::log("user is",splits[1],ip);

    sd.setUser(splits[1],ip);

    sd.printUsers();
    
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

void handleConnection(int& fd,ServerData& data)
{
    Utils::log("do stuff");


    Utils::log("connected with FD",fd);

    std::string msg;
    if(!NetCommon::recvMsg(fd,msg))
    {
        Utils::log("recv Failed!");
    }

    processSelfId(fd,msg,data);

    sendList(fd);

    if(!NetCommon::recvMsg(fd,msg))
    {
        Utils::log("recv Failed!");
    }

    Utils::log("Received",msg);
}

int main()
{
    NetServer server;

    server.startServer(SERVER_PORT);

    ServerData data;

    while(true)
    {
        int fd; 
        if(!server.acceptConnection(fd))
        {
            Utils::log("Connection Failed!");
        }
        else
        {
            std::thread connectionThread(&handleConnection,std::ref(fd),std::ref(data));
            connectionThread.detach();
        }
    }

    return 0;
}
