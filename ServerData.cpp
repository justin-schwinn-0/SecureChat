#include "ServerData.h"

ServerData::ServerData()
{
}

ServerData::~ServerData()
{
}

void ServerData::setUser(const int fd,const std::string& user, const std::string& ip)
{
    UserData newUser= {fd,ip,NONE,false};

    userData[user] = newUser;
}

const UserData& ServerData::getUser(const std::string& name)
{
    return userData[name];
}

void ServerData::printUsers()
{
    for(auto& pair : userData)
    {
        Utils::log(pair.first,pair.second.ip,pair.second.commRequest);
    }
}

bool ServerData::makeRequest(std::string requester ,std::string requested)
{
    if(requester == requested)
    {
        return false;
    }
    if(userData.find(requested) != userData.end() && getRequester(requested) == NONE && !userData[requested].busy)
    {
        userData[requested].commRequest = requester;
        return true;
    }
    else
    {
        printRequests();
        return false;
    }
}



std::string ServerData::getRequester(const std::string& user)
{
    return userData[user].commRequest;
}

void ServerData::printRequests()
{
    for(auto& pair : userData)
    {
        Utils::log(pair.first," requested by ",pair.second.commRequest);
    }
}

std::vector<std::string> ServerData::getListForUser(const std::string& user)
{
    std::vector<std::string> list;

    for(auto u : userData)
    {
        std::string str = u.first;

        //Utils::log(user,getRequester(u.first));
        if(u.second.busy)
        {
            str+= " BUSY";
        }
        else if(user == u.first && getRequester(u.first) != NONE)
        {
            str += " COM REQ FROM "+ u.second.commRequest;
        }

        list.push_back(str);
    }

    return list;
}

void ServerData::setBusy(const std::string& user)
{
    userData[user].busy = true;
}
