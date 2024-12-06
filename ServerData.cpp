#include "ServerData.h"

ServerData::ServerData()
{
}

ServerData::~ServerData()
{
}

void ServerData::setUser(const std::string& user, const std::string& ip)
{
    userData[user] = ip;
}

std::string ServerData::getUser(const std::string& name)
{
    return userData[name];
}


void ServerData::printUsers()
{
    for(auto& pair : userData)
    {
        Utils::log(pair.first,pair.second);
    }
}
