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

const std::map<int,std::string> commands = 
{
    {LIST,"list"},
    {CON,"connect"},
    {EXIT,"exit"} 
};

void listCommands()
{
    std::string cl = "";
    for(auto c : commands)
    {
        cl += c.second + " ";
    }

    Utils::log("Commands:", cl);
}

bool handleCommand(int& serverFd,const std::string& cmd,const std::string& name)
{
    auto segments = Utils::split(cmd," ");

    int msgId =-1;

    for(auto c : commands)
    {
        if(c.second == segments[0])
        {
            msgId = c.first;
        }
    }

    if(msgId == -1)
    {
        Utils::log("Unknown Command!");
        return false;
    }

    //Utils::log("command:",msgId);

    switch(msgId)
    {
        case LIST:
            NetCommon::sendPayload(serverFd,Message(msgId,{name}));
            break;
        case CON:
            if(segments.size() == 2)
            {
                NetCommon::sendPayload(serverFd,Message(msgId,{name,segments[1]}));
            }
            else
            {
                Utils::log("incorrect command format!");
                return false;
            }
            break;
        default:
            Utils::log("Unknown command to send!");
            return false;
    }

    return true;

}

std::string getInput(std::string prompt)
{
    std::string out;

    Utils::log(prompt);
    std::getline(std::cin, out);
    return out;
}


void handleList(std::vector<std::string> list)
{
    int i = 1;
    for(auto s : list)
    {
        Utils::log(std::to_string(i)+".",s);
        i++;
    }
}

bool handleMsg(int& fd,const std::string& str)
{
    auto msg = Message(str);

    switch(msg.msgId)
    {
        case LIST:
            handleList(msg.payload);
            break;
        default:
            Utils::log("Unknown msg_id:",msg.msgId,"\n");
            return true;
    }

    return false;
}

int main() 
{
    std::string name = getInput("Who are you");
    int centralServerFd;

    // Create an SCTP socket
    if(!NetCommon::connectTo(centralServerFd,SERVER_IP,SERVER_PORT))
    {
        Utils::log("Could not connect");
        return 1;
    }

    NetCommon::sendPayload(centralServerFd,Message(ID,{name}));

    while(true)
    {
        Utils::log("Listening to server");
        std::string msg;
        if(!NetCommon::recvMsg(centralServerFd,msg))
        {
            Utils::log("recv Failed!");
        }

        handleMsg(centralServerFd,msg);

        listCommands();
        bool cmdSuccess = false;
        do
        {
            cmdSuccess = handleCommand(centralServerFd,getInput(">"),name);
        }
        while(!cmdSuccess);
    }
    Utils::log("exiting");

    //NetCommon::sendPayload(centralServerFd,Message(CON,{name,connectToUser}));

    close(centralServerFd);
    return 0;
}
