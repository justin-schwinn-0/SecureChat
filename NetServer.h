#ifndef NETSERVER_H
#define NETSERVER_H

#include <string>

class NetServer
{
public:
    NetServer();
    ~NetServer();

    bool startServer(int port);

    bool acceptConnection(int& fd);
    
    void printFd();

private:

    int mListenFd;

};

#endif
