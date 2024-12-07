#include "Utils.h"
#include "NetServer.h"
#include "NetCommon.h"

#include "ServerData.h"

#include <thread>

const int SERVER_PORT = 5000;

void processSelfId(int& fd,const Message& msg,ServerData& sd)
{
    std::string ip = NetCommon::getIp(fd);

    Utils::log("user is",msg.payload[0],ip);

    sd.setUser(msg.payload[0],ip);

    sd.printUsers();
}

void sendList(int& fd,ServerData& sd,const std::string& requestingUser)
{
    std::vector<std::string> msg;
    for(auto s: sd.getListForUser(requestingUser))
    { 
        msg.push_back(s+":");
    }


    NetCommon::sendPayload(fd,Message(LIST,msg));
}

void connectCtC(int& client1,const Message& msg,ServerData& sd)
{
    Utils::log("connecting",msg.payload[0], "to",msg.payload[1]);
        
    if(sd.makeRequest(msg.payload[0],msg.payload[1]))
    {
        Utils::log("request made");
        sd.printRequests();
    }
    else
    {
        Utils::log("Cannot allow request");
    }
}

bool handleMsg(int& fd,const std::string& str,ServerData& data)
{
    auto msg = Message(str);
    

    switch(msg.msgId)
    {
        case ID:
            processSelfId(fd,msg,data);
            sendList(fd,data,msg.payload[0]);
            break;
        case LIST:
            sendList(fd,data,msg.payload[0]);
            break;
        case CON:
            connectCtC(fd,msg,data);
            break;
        default:
            Utils::log("Unknown msg_id:",msg.msgId,"\n");
            return true;
    }

    return false;
}

void handleConnection(int fd,ServerData& data)
{
    Utils::log("connected with FD",fd);

    bool hasExited = false;
    do
    {
        std::string msg;
        if(!NetCommon::recvMsg(fd,msg))
        {
            Utils::log("recv Failed!");
        }

        if(msg.empty())
        {
            hasExited= true;
        }
        else
        {
            Utils::log("Received",msg);
            hasExited = handleMsg(fd,msg,data);
        }

        Utils::log(fd,"has exited:",hasExited);

    }while(!hasExited);

    Utils::log("exiting for fd",fd);
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
            std::thread connectionThread(&handleConnection,fd,std::ref(data));
            connectionThread.detach();
        }
    }

    return 0;
}
