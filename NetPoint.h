#ifndef NETSERVER_H
#define NETSERVER_H

class NetServer
{
public:
    NetServer();
    ~NetServer();

    bool startServer(int port);

    bool acceptConnection(int& fd);

private:

    int mListenFd;

};

#endif
