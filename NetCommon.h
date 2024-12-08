#ifndef NETCOMMON_H
#define NETCOMMON_H

#include <string>
#include <vector>


const int ID = 1;
const int LIST = 2;
const int CON = 3;
const int EXIT = 4;


const int ACCEPT = 5;
const int REJECT = 6;

const int OPEN_SERVER = 7;
const int CONNECT_TO = 8;

const int CHAT_MSG = 9;



const std::string DELIM = ":";

struct Message
{
    int msgId = -1;
    std::vector<std::string> payload;

    Message();
    Message(int id,std::vector<std::string> p):
        msgId(id),
        payload(p)
    {}
    Message(std::string str);
};


class NetCommon
{
public:

    static bool recvMsg(const int& fd, std::string& out);

    static bool sendMsg(const int& fd, const std::string& in);

    static bool connectTo(int& fd, const std::string& ip, const int port);

    static std::string getIp(const int& fd);

    static bool sendPayload(const int& fd, const Message& message);

};

#endif
