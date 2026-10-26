#include "Server.hpp"

Server::Server(std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end)
{
    retrieve_info(it, end);
}

Server::Server(Server const &other)
{
}

Server &Server::operator=(Server const &other)
{
}

Server::~Server()
{
}

void    Server::retrieve_info(std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end )
{
    if (*it != "{")
        throw (InvalidTokenAfterServer());
    //il faudra ensuite faire une extraction des blocs de location
    //verifier comment c'est compatible avec l'expression while it != end
    while (it != end)
    {
        if (*it == "location")
        {
            ++it;
            Location location_block(it, end);

            location_blocks.push_back(location_block);
        }
        it++;
    }
}