#include "Server.hpp"

Server::Server(IT &it, IT end)
{
    retrieve_info(it, end);
}

Server::Server(Server const &other)
{
    server = other.server;
}

Server &Server::operator=(Server const &other)
{
    if (this != &other)
    {
        server = other.server;
    }
    return (*this);
}

Server::~Server()
{
}

void    Server::retrieve_info(std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end )
{
    std::string const available_modules[] = {"server_name", "root", "error_page", \
        "client_max_body_size", "listen", "location"};
    void (Server::*function[])(IT &, IT) = {&Server::server_name, &Server::root,\
    &Server::error_pages, &Server::client_size, &Server::listen, &Server::location};
    if (*it++ != "{")
        throw (ErrorServerBlock("No { after server declaration"));
    //il faudra ensuite faire une extraction des blocs de location
    //verifier comment c'est compatible avec l'expression while it != end
    while (!(it == end || *it == "}"))
    {
        for (int i = 0; i < 6; i++)
        {
            if (*it == available_modules[i])
            {
                ++it;
                (this->*function[i])(it, end);
                break;
            }
            if (i == 5)
                throw(ErrorServerBlock("Invalid method in server block"));
        }
    }
    if (*it != "}")
        throw(ErrorServerBlock("server blocks must end in }"));
    it++;
}

void    Server::server_name(IT &it, IT end)
{
    while (!(it == end || *it == ";"))
    {
        server.server_names.push_back(*it);
        ++it;
    }
    if (it == end)
        throw (ErrorServerBlock("Missing a semi-colon in server_name"));
    ++it;
}

void    Server::root(IT &it, IT end)
{
    if ((it + 1) == end || *(it + 1) != ";")
        throw(ErrorServerBlock("root methods only accept one arguement and must end in semi-colon"));
    server.server_repo = *it;
    it = it + 2;
}

void    Server::error_pages(IT &it, IT end)
{
    int code;

    if ((it + 1) == end || (it + 2) == end || *(it + 1) == ";" || *(it + 2 ) != ";")
        throw(ErrorServerBlock("error_logs must have two arguments and a semi-colon"));
    code =  atoi((*it).c_str());
    server.error_files[code] =  *(it + 1);
    it = it + 3;
}


void    Server::client_size(IT &it, IT end)
{
    if ((it + 1) == end || *(it + 1) != ";")
        throw(ErrorServerBlock("client_max_body_size requires one argument and a semi-colon"));
    server.max_body_size = atoi((*it).c_str());
    it = it + 2;
}

void    Server::listen(IT &it, IT end)
{
     if ((it + 1) == end || *(it + 1) != ";")
        throw(ErrorServerBlock("listen requires one argument and a semi-colon"));
    server.open_ports.push_back(atoi((*it).c_str()));
    it  = it + 2;
   
}

void    Server::location(IT &it, IT end)
{
    Location    location_block(it, end);

    server.location_blocks.push_back(location_block.get_location());
}

t_server const & Server::get_server() const
{
    return (this->server);
}