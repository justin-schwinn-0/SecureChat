#include "Utils.h"
#include "NetServer.h"

const int SERVER_PORT = 5000;

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
    if(!server.recvMsg(fd,msg))
    {
        Utils::log("recv Failed!");

    }

    Utils::log("Received",msg);

    return 0;
}
