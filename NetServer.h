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
    
    bool recvMsg(const int& fd, std::string& out);

    bool sendMsg(const int& fd, const std::string& in);

    void printFd();



private:

    int mListenFd;

};

#endif
