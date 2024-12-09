#include "NetCommon.h"

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

#include <openssl/evp.h>
#include <openssl/err.h>


#include "Utils.h"

#include "Crypto.h"

const int BUFFER_SIZE = 1024;


Message::Message(std::string str)
{
    auto splits = Utils::split(str,DELIM);

    msgId = Utils::strToInt(splits[0]);

    payload = std::vector<std::string>(splits.begin()+1,splits.end());

}

bool NetCommon::recvMsg(const int& fd, std::string& out)
{
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    struct sctp_sndrcvinfo sndrcvinfo;
    int flags=0;

    ssize_t bytes_received = sctp_recvmsg(fd, buffer, BUFFER_SIZE, NULL,0,&sndrcvinfo,&flags);


    out = buffer;
    return true;
}


bool NetCommon::secRecvMsg(const int& fd, std::string& out)
{
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    struct sctp_sndrcvinfo sndrcvinfo;
    int flags=0;

    ssize_t bytes_received = sctp_recvmsg(fd, buffer, BUFFER_SIZE, NULL,0,&sndrcvinfo,&flags);


    std::string msg = buffer;
    out = Crypto::custom_decrypt(DEFAULT_KEY,msg,DEFAULT_IV);
    Utils::log("got",out);
    Utils::log(out.size(),bytes_received,msg.size());
    return true;
}

bool NetCommon::recvMsg(const int& fd, std::vector<unsigned char>& out)
{
    unsigned char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    struct sctp_sndrcvinfo sndrcvinfo;
    int flags=0;

    ssize_t bytes_received = sctp_recvmsg(fd, buffer, BUFFER_SIZE, NULL,0,&sndrcvinfo,&flags);


    out = std::vector<unsigned char>(buffer,buffer+bytes_received);
    return true;
}

bool NetCommon::sendMsg(const int& fd, const std::string& in)
{
    //Utils::log("send", in);
    int ret = send(fd, in.c_str(), in.size(), 0);
    if(ret < 0)
    {
        Utils::log("could not send message:",in,ret);
        return false;
    }

    return true;
}

bool NetCommon::secSendMsg(const int& fd, const std::string& in)
{
    Utils::log("sending:",in, in.size());
    std::string msg = Crypto::custom_encrypt(DEFAULT_KEY,in,DEFAULT_IV);
    int ret = send(fd, msg.c_str(), msg.size(), 0);
    if(ret < 0)
    {
        Utils::log("could not send message:",in,ret);
        return false;
    }

    Utils::log("sent:",ret,in,msg);

    return true;
}

bool NetCommon::sendMsg(const int& fd, const std::vector<unsigned char> in)
{
    //Utils::log("send", in);
    int ret = send(fd, in.data(), in.size(), 0);
    if(ret < 0)
    {
        Utils::log("could not send message unsigned char message",ret);
        return false;
    }

    return true;
}

bool NetCommon::connectTo(int& fd, const std::string& ip, const int port)
{
    // Create an SCTP socket
    fd = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);
    if (fd < 0) {
        perror("Socket creation failed");
        return false;
    }

    sockaddr_in server_addr{};
    // Configure the server address
    server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr);
    server_addr.sin_port = htons(port);

    // Connect to the server
    if (connect(fd, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(fd);
        return false;
    }

    return true;
}

std::string NetCommon::getIp(const int& fd)
{
    sockaddr_in addr{};

    socklen_t addr_size = sizeof(struct sockaddr_in);
    int res = getpeername(fd, (struct sockaddr *)&addr, &addr_size);

    std::string ip = inet_ntoa(addr.sin_addr);

    return ip;
}


bool NetCommon::sendPayload(const int& fd,const Message& message)
{
    std::string msg = std::to_string(message.msgId) + DELIM;

    for(int i = 0; i < message.payload.size();i++)
    {
        msg += message.payload[i];

        if(i != message.payload.size()-1)
        {
            msg+= DELIM;
        }
    }

    return NetCommon::sendMsg(fd,msg);
}

bool NetCommon::secSendPayload(const int& fd,const Message& message)
{
    std::string msg = std::to_string(message.msgId) + DELIM;

    for(int i = 0; i < message.payload.size();i++)
    {
        msg += message.payload[i];

        if(i != message.payload.size()-1)
        {
            msg+= DELIM;
        }
    }

    return NetCommon::secSendMsg(fd,msg);
}
