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


bool NetCommon::secRecvMsg(const int& fd, std::string& out,const std::string& key)
{
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    struct sctp_sndrcvinfo sndrcvinfo;
    int flags=0;

    ssize_t bytes_received = sctp_recvmsg(fd, buffer, BUFFER_SIZE, NULL,0,&sndrcvinfo,&flags);

    std::string msgRx = std::string(buffer,bytes_received);

    std::string cipherText = msgRx.substr(0,msgRx.size()-16);
    std::string hash = msgRx.substr(msgRx.size()-16);

    Utils::log("Message and hash",cipherText.size(),hash.size());

    std::string newhash = Crypto::md5_encrypt(key,"0000000000000000",cipherText);
        Utils::log("key:",key);

    if(newhash != hash)
    {
        Utils::log("Message integrity failure!",newhash,hash,key);
        return false;
    }

    out = Crypto::custom_decrypt(key,cipherText,DEFAULT_IV);

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

bool NetCommon::secSendMsg(const int& fd, const std::string& in,const std::string& key)
{
    std::string msg = Crypto::custom_encrypt(key,in,DEFAULT_IV);
    std::string msgHash = Crypto::md5_encrypt(key,"0000000000000000",msg);

    std::string msgToSend = msg+msgHash;
    int ret = send(fd, msgToSend.c_str(), msgToSend.size(), 0);
    if(ret < 0)
    {
        Utils::log("could not send message:",in,ret);
        return false;
    }

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

bool NetCommon::secSendPayload(const int& fd,const Message& message,const std::string& key)
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

    return NetCommon::secSendMsg(fd,msg,key);
}
