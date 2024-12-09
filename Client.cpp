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
#include "NetServer.h"
#include "Utils.h"
#include "Crypto.h"

#include <openssl/err.h>

#include "ttmath/ttmath.h"

const std::string SERVER_IP = "127.0.0.1";
const int SERVER_PORT = 5000;
const int CHAT_PORT = 5001;
const int BUFFER_SIZE = 1024;

const int HELP = 1001;

struct commandData
{
    std::string name;
    int length;
};

const std::map<int,commandData> commands = 
{
    {LIST,{"list",1}},
    {CON,{"connect",2}},
    {EXIT,{"exit",1}},
    {ACCEPT,{"accept",1}},
    {REJECT, {"reject",1}},
    {HELP,{"help",1}}
};

void listCommands()
{
    std::string cl = "";
    for(auto c : commands)
    {
        cl += c.second.name + " ";
    }

    Utils::log("Commands:", cl);
}

bool handleCommand(int& serverFd,const std::string& cmd,const std::string& name)
{
    auto segments = Utils::split(cmd," ");

    int msgId =-1;

    for(auto c : commands)
    {
        if(c.second.length == segments.size()&& 
           c.second.name == segments[0])
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
            NetCommon::secSendPayload(serverFd,Message(msgId,{name}));
            break;
        case CON:
            NetCommon::secSendPayload(serverFd,Message(msgId,{name,segments[1]}));
            break;
        case ACCEPT:
            NetCommon::secSendPayload(serverFd,Message(msgId,{name}));
            break;
        case REJECT:
            NetCommon::secSendPayload(serverFd,Message(msgId,{name}));
            break;
        case HELP:
            listCommands();
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

std::string getInlineInput(std::string prompt)
{
    std::string out;

    std::cout << prompt;
    std::getline(std::cin, out);
    return out;
}


void handleList(std::vector<std::string> list)
{
    int i = 1;

    Utils::log("Online Users:");
    for(auto s : list)
    {
        Utils::log(std::to_string(i)+".",s);
        i++;
    }
}

void sendExitChatCmd(const int fd)
{
    if(!NetCommon::secSendPayload(fd,Message(EXIT,{})));
    {
        Utils::log("could not send exit Cmd!");
    }
}

bool handleChatMsg(const std::string& str)
{
    auto msg = Message(str);

    switch(msg.msgId)
    {
        case CHAT_MSG:
            Utils::log(">>>",msg.payload[0]);
            break;
        case EXIT:
            Utils::log("Other client exited chat!");
            return true;
        default:
            Utils::log("ERROR: unknown message");
            return true;
    }

    return false;
}


void chatLoop(int fd)
{
    Utils::log("Entering Chat with",NetCommon::getIp(fd));
    Utils::log("EXIT to exit chat\n");

    bool hasExited = false;
    while(!hasExited)
    {
        std::string str;
        if(!NetCommon::secRecvMsg(fd,str))
        {
            Utils::log("recv Failed!");
            sendExitChatCmd(fd);
            hasExited = true;
        }
        else
        {
            hasExited = handleChatMsg(str);
        }


        std::string msgOut = getInlineInput("you >");

        if(msgOut == "EXIT")
        {
            sendExitChatCmd(fd);
            hasExited = true;
        }
        else
        {
            if(!NetCommon::secSendPayload(fd,Message(CHAT_MSG,{msgOut})))
            {
                Utils::log("Could not send message!");
                sendExitChatCmd(fd);
                hasExited = true;
            }
        }
    }
    Utils::log("Exiting chat");
}

void openServer()
{
    Utils::log("Opening server");
    NetServer srv;
    srv.startServer(CHAT_PORT);

    int fd; 
    if(!srv.acceptConnection(fd))
    {
        Utils::log("Connection Failed!");
        return;
    }

    bool hasExited = false;

    Utils::log("connected to other client",fd);


    std::string firstMessage = getInput("Send first Message:");

    NetCommon::secSendPayload(fd,Message(CHAT_MSG,{firstMessage}));

    chatLoop(fd);

}

void connectToOtherClient(const Message& msg)
{
    //Utils::log("connect to other client");

    std::string otherClientIp = msg.payload[0];
    int otherClientFd;
    if(!NetCommon::connectTo(otherClientFd,otherClientIp,CHAT_PORT))
    {
        Utils::log("Could not connect");
        return;
    }

    //Utils::log("connected to other client");

    chatLoop(otherClientFd);
}

bool handleMsg(int& fd,const std::string& str)
{
    auto msg = Message(str);

    switch(msg.msgId)
    {
        case LIST:
            handleList(msg.payload);
            break;
        case OPEN_SERVER:
            openServer();
            break;
        case CONNECT_TO:
            connectToOtherClient(msg);
            break;
        case REJECT:
            Utils::log("No Connection to Accept!");
            break;
        default:
            Utils::log("Unknown msg:",str,"\n");
            return true;
    }

    return false;
}

bool handleAuth(const int fd, ttmath::UInt<3>& nonce,const std::string& name)
{
    if(!NetCommon::secSendPayload(fd,Message(ID,{name})))
    {
        Utils::error("Could not send ID!");
        return false;
    }

    std::vector<unsigned char> signedNonce;
    if(!NetCommon::recvMsg(fd,signedNonce))
    {
        Utils::error("Could not read Nonce!");
        return false;
    }

    Utils::log("Signed Nonce Str:",signedNonce.size());
    
    Crypto::printEncryption(signedNonce);


    auto privKey = Crypto::load_private_key_from_file("Keys/"+name+".pem"); 

    std::vector<unsigned char> decryptedNonce(RSA_size(privKey));
    int decryptedBytes = RSA_private_decrypt(
                     signedNonce.size(),
                     signedNonce.data(),
                     decryptedNonce.data(),
                     privKey,
                     RSA_PKCS1_OAEP_PADDING);

    if(decryptedBytes == -1)
    {
        Utils::log(ERR_error_string(ERR_get_error(), nullptr));
    }



    Crypto::printEncryption(decryptedNonce);


    nonce.SetZero();
    for(int i = 0; i < decryptedNonce.size();i++)
    {
        nonce.AddInt(decryptedNonce[i] << (i * 8));
    }

    Utils::log(decryptedBytes,"Decrypted Nonce:", nonce);
}

int main(int argc, char** argv) 
{
    std::string serverIp = "127.0.0.1";
    if(argc != 2)
    {
        Utils::log("No IP given!");
        return -1;
    }
    else
    {
        serverIp = argv[1];
    }

    std::string name;
    while(name.empty())
    {
        name = getInput("Who are you");
        if(name.empty())
        {
            Utils::log("Cannot have empty name!");
        }
    }
    int centralServerFd;

    // Create an SCTP socket
    if(!NetCommon::connectTo(centralServerFd,serverIp,SERVER_PORT))
    {
        Utils::log("Could not connect");
        return 1;
    }

    //ttmath::UInt<3> nonce1024;

    //handleAuth(centralServerFd,nonce1024,name);
    if(!NetCommon::secSendPayload(centralServerFd,Message(ID,{name})))
    {
        Utils::error("Could not send ID!");
        return false;
    }


    while(true)
    {
        //Utils::log("Listening to server");
        std::string msg;
        if(!NetCommon::recvMsg(centralServerFd,msg))
        {
            Utils::log("recv Failed!");
        }

        handleMsg(centralServerFd,msg);

        bool cmdSuccess = false;
        do
        {
            cmdSuccess = handleCommand(centralServerFd,getInlineInput(">"),name);
        }
        while(!cmdSuccess);
    }
    Utils::log("exiting");

    //NetCommon::secSendPayload(centralServerFd,Message(CON,{name,connectToUser}));

    close(centralServerFd);
    return 0;
}
