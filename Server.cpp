#include "Utils.h"
#include "NetServer.h"
#include "NetCommon.h"

#include "ServerData.h"

#include <thread>

#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/err.h>

#include "ttmath/ttmath.h"

#include "Crypto.h"

#include <random>

const int SERVER_PORT = 5000;


template<long unsigned int T>
void generateRandom(ttmath::UInt<T>& num)
{
    std::random_device rd;  
    std::mt19937 generator(rd()); 
    std::uniform_int_distribution<uint32_t> distribution(0, 0xFFFFFFFF);

    for(int i = 0; i < T; i++)
    {
        num.table[i] = distribution(generator);
    }

    //Utils::log("generated",num);
}

bool authenticate(const int& fd,const Message& msg,ServerData& sd)
{
    // do auth
    ttmath::UInt<3> nonce1024;

    generateRandom(nonce1024);

    //nonce1024 = 6;

    Utils::log("Nonce:",nonce1024);

    std::string name = msg.payload[0];

    auto pubKey = Crypto::load_public_key_from_file("Keys/"+name+".pem.pub"); 

    Utils::log("read public key of",name);

    std::vector<unsigned char> unsignedNonce;

    for(int i = 0; i < nonce1024.Size();i++)
    {
        uint64_t segment = nonce1024.table[i];

        for(int j = 0; j < 4 ;j++)
        {
            unsignedNonce.push_back(static_cast<unsigned char>(segment & 0xFF));
            segment>>=8;
        }
        Crypto::printEncryption(unsignedNonce);
    }

    std::vector<unsigned char> signedNonce(RSA_size(pubKey));
    int encryptedBytes = RSA_public_encrypt(
                     nonce1024.Size()*8,
                     reinterpret_cast <const unsigned char*>(nonce1024.table),
                     signedNonce.data(),
                     pubKey,
                     RSA_PKCS1_OAEP_PADDING);

    if(encryptedBytes == -1)
    {
        Utils::log(ERR_error_string(ERR_get_error(), nullptr));
    }

    if(!NetCommon::sendMsg(fd,signedNonce))
    {
        Utils::error("Could not send Nonce!");
    }

    Crypto::printEncryption(signedNonce);

    Utils::log("encrypted", encryptedBytes);

    RSA_free(pubKey);
    return false;
}

void processSelfId(const int& fd,const Message& msg,ServerData& sd)
{
    std::string ip = NetCommon::getIp(fd);

    authenticate(fd,msg,sd);

    Utils::log("user is",msg.payload[0],ip);

    sd.setUser(fd,msg.payload[0],ip);

    sd.printUsers();
}

void sendList(int& fd,ServerData& sd,const std::string& requestingUser)
{
    std::vector<std::string> msg;
    for(auto s: sd.getListForUser(requestingUser))
    { 
        msg.push_back(s+":");
    }

    NetCommon::sendPayload(fd,Message(LIST,msg));
}

void connectCtC(int& client1,const Message& msg,ServerData& sd)
{
    Utils::log("connecting",msg.payload[0], "to",msg.payload[1]);
        
    if(sd.makeRequest(msg.payload[0],msg.payload[1]))
    {
        Utils::log("request made");
        sd.printRequests();
    }
    else
    {
        Utils::log("Cannot allow request");
    }
}

void clientAcceptsConnection(int& client2,const Message& msg,ServerData& sd)
{
    // tell client 2 to open server
    // tell client 1 to connect to client 2 server
    // mark client 1 and 2 as busy
    
    //User (client 2) accepted connection to (client 1)
    auto client1Name = sd.getRequester(msg.payload[0]);
    auto client2Name = msg.payload[0];

    if(client1Name == NONE)
    {
        Utils::log("Client",client2Name,"has no requests!");
        NetCommon::sendPayload(client2,Message(REJECT,{}));
        return;

    }
    Utils::log("User",client2Name,"accepted connection to",client1Name);

    // send client 2 opens server
    NetCommon::sendPayload(client2,Message(OPEN_SERVER,{}));


    int client1Fd = sd.getUser(client1Name).fd;
    std::string client2Ip = NetCommon::getIp(client2);

    // send client 1 cmd to connect with IP
    NetCommon::sendPayload(client1Fd,Message(CONNECT_TO,{client2Ip}));
    
}      

bool handleMsg(int& fd,const std::string& str,ServerData& data)
{
    auto msg = Message(str);
    

    switch(msg.msgId)
    {
        case ID:
            processSelfId(fd,msg,data);
            sendList(fd,data,msg.payload[0]);
            break;
        case LIST:
            sendList(fd,data,msg.payload[0]);
            break;
        case CON:
            connectCtC(fd,msg,data);
            break;
        case ACCEPT:
            clientAcceptsConnection(fd,msg,data);
            break;
        default:
            Utils::log("Unknown msg_id:",msg.msgId,"\n");
            return true;
    }

    return false;
}


void handleConnection(int fd,ServerData& data)
{
    Utils::log("connected with FD",fd);


    bool hasExited = false;
    do
    {
        std::string msg;
        if(!NetCommon::recvMsg(fd,msg))
        {
            Utils::log("recv Failed!");
        }

        if(msg.empty())
        {
            hasExited= true;
        }
        else
        {
            //Utils::log("Received",msg);
            hasExited = handleMsg(fd,msg,data);
        }

        //Utils::log(fd,"has exited:",hasExited);

    }while(!hasExited);
}

int main()
{

    ttmath::UInt<4> bigInt1 = "123456789123456789";

    NetServer server;

    server.startServer(SERVER_PORT);

    ServerData data;

    while(true)
    {
        int fd; 
        if(!server.acceptConnection(fd))
        {
            Utils::log("Connection Failed!");
        }
        else
        {
            std::thread connectionThread(&handleConnection,fd,std::ref(data));
            connectionThread.detach();
        }
    }

    return 0;
}
