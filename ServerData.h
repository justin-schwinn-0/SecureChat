#ifndef SERVER_DATA_H
#define SERVER_DATA_H

#include "Utils.h"

#include <string>

class ServerData
{
public:
    ServerData();
    ~ServerData();

    void setUser(const std::string& user, const std::string& ip);

    std::string getUser(const std::string& user);

    

private:
    std::map<std::string, std::string> userData;

};

#endif
