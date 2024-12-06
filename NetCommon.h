#ifndef NETCOMMON_H
#define NETCOMMON_H

#include <string>

class NetCommon
{
public:

    static bool recvMsg(const int& fd, std::string& out);

    static bool sendMsg(const int& fd, const std::string& in);

    static bool connectTo(int& fd, const std::string& ip, const int port);

    static std::string getIp(int& fd)

};

#endif
