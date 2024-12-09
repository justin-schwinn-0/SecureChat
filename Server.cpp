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

bool authenticate(const int& fd,const Message& msg,ServerData& sd,ttmath::UInt<8>& nonce)
{
    // do auth
    ttmath::UInt<8> nonce1024;

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
    }

    std::vector<unsigned char> signedNonce(RSA_size(pubKey));
    int encryptedBytes = RSA_public_encrypt(
                     nonce1024.Size()*4,
                     reinterpret_cast <const unsigned char*>(nonce1024.table),
                     signedNonce.data(),
                     pubKey,
                     RSA_PKCS1_OAEP_PADDING);

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


    Crypto::charVecToNum(decryptedNonce,nonce);



    if(encryptedBytes == -1)
    {
        Utils::log(ERR_error_string(ERR_get_error(), nullptr));
    }

    if(!NetCommon::sendMsg(fd,signedNonce))
    {
        Utils::error("Could not send Nonce!");
    }

    std::string strMsg;
    if(!NetCommon::secRecvMsg(fd,strMsg,nonce.ToString()))
    {
        Utils::log("recv Failed!");
    }

    auto resp = Message(strMsg);



    if(resp.msgId == CHALLENGE_RESP)
    {
        ttmath::UInt<8> otherNonce = resp.payload[0];

        if((otherNonce-1) == nonce)
        {
            Utils::log("Authenticated!");
            
            nonce+=2;
            if(!NetCommon::secSendPayload(fd,Message(ACCEPT_AUTH,{}),nonce.ToString()))
            {
                Utils::log("auth failed");
            }
            return true;

        }
        else
        {
            Utils::log("Authentication fail!",otherNonce,nonce);

        }
    }

    RSA_free(pubKey);
    return false;
}

bool processSelfId(const int& fd,const Message& msg,ServerData& sd, ttmath::UInt<8>& key)
{
    std::string ip = NetCommon::getIp(fd);

    if(!authenticate(fd,msg,sd,key))
    {
        return false;
    }

    Utils::log("user is",msg.payload[0],ip);

    sd.setUser(fd,msg.payload[0],ip);

    sd.printUsers();

    return true;
}

void sendList(int& fd,ServerData& sd,const std::string& requestingUser,const std::string& key)
{
    std::vector<std::string> msg;
    for(auto s: sd.getListForUser(requestingUser))
    { 
        msg.push_back(s+":");
    }

    NetCommon::secSendPayload(fd,Message(LIST,msg),key);
}

void connectCtC(int& client1,const Message& msg,ServerData& sd,const std::string& key)
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

        NetCommon::secSendPayload(client1,Message(REJECT,{}),key);
    }
}

void clientAcceptsConnection(int& client2,const Message& msg,ServerData& sd,const std::string& key)
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
        NetCommon::secSendPayload(client2,Message(REJECT,{}),key);
        return;

    }
    Utils::log("User",client2Name,"accepted connection to",client1Name);

    sd.setBusy(client1Name);
    sd.setBusy(client2Name);

    ttmath::UInt<8> ck;
    generateRandom(ck);
    std::string clientKey = ck.ToString();
    // send client 2 opens server
    NetCommon::secSendPayload(client2,Message(OPEN_SERVER,{clientKey}),key);

    Utils::log(__FUNCTION__,__LINE__);

    int client1Fd = sd.getUser(client1Name).fd;
    std::string client2Ip = NetCommon::getIp(client2);
    Utils::log(__FUNCTION__,__LINE__);

    // send client 1 cmd to connect with IP
    NetCommon::secSendPayload(client1Fd,Message(CONNECT_TO,{client2Ip,clientKey}),key);
    Utils::log(__FUNCTION__,__LINE__);
    
}      

bool handleMsg(int& fd,const std::string& str,ServerData& data,const std::string& key)
{
    auto msg = Message(str);
    

    switch(msg.msgId)
    {
        case LIST:
            sendList(fd,data,msg.payload[0],key);
            break;
        case CON:
            connectCtC(fd,msg,data,key);
            break;
        case ACCEPT:
            clientAcceptsConnection(fd,msg,data,key);
            break;
        default:
            Utils::log("Unknown msg_id:",msg.msgId,"\n",str);
            return true;
    }

    return false;
}


void handleConnection(int fd,ServerData& data)
{
    Utils::log("connected with FD",fd);
    

    std::string msg;
    if(!NetCommon::recvMsg(fd,msg))
    {
        Utils::log("recv Failed!");
    }


    Utils::log(msg);
    auto IdMsg = Message(msg);

    ttmath::UInt<8> key;
    if(!processSelfId(fd,IdMsg,data,key))
    {
        return;
    }
    else
    {
        Utils::log("pint key",key.ToString());
        sendList(fd,data,IdMsg.payload[0],key.ToString());
    }


    bool hasExited = false;
    do
    {
        std::string msg;
        if(!NetCommon::secRecvMsg(fd,msg,key.ToString()))
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
            hasExited = handleMsg(fd,msg,data,key.ToString());
        }

        //Utils::log(fd,"has exited:",hasExited);

    }while(!hasExited);
}

int main()
{

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
