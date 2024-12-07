#ifndef SERVER_DATA_H
#define SERVER_DATA_H

#include "Utils.h"

#include <string>

const std::string NONE= "none";

struct UserData
{
    std::string ip;
    std::string commRequest;
};

class ServerData
{
public:
    ServerData();
    ~ServerData();

    void setUser(const std::string& user, const std::string& ip);

    const UserData& getUser(const std::string& user);

    void printUsers();
    void printRequests();

    bool makeRequest(std::string requester,std::string requested);

    // gets any request going to the user
    // retruns bob when bob wants to talk to user
    std::string getRequester(const std::string& user);

    std::vector<std::string> getListForUser(const std::string& user);
    

private:
    std::map<std::string, UserData> userData;
};

#endif
