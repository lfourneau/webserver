#include "Server.hpp"
#include <sys/stat.h>

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

Server::~Server() throw()
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
    if (it == end || *it != "}")
        throw(ErrorServerBlock("server blocks must end in }"));
    it++;
    resolve_error_pages();
    resolve_location_paths();
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

//les errors codes plus fichiers permettent de dire a nginx de servir un fichier en particulier
//en cas d'erreur dans ce projet on s'en sert differemment
void    Server::error_pages(IT &it, IT end)
{
    std::vector<int> codes;

    while (it != end && *it != ";" && (it + 1) != end && *(it + 1) != ";")
    {
        int code;

        if (it->find_first_not_of("0123456789") != std::string::npos)
            throw(ErrorServerBlock("error_page: invalid status code " + *it));
        transform_to_num(std::string(*it), code);
        if (code < 300 || code > 599)
            throw(ErrorServerBlock("error_page: status code must be between 300 and 599"));
        codes.push_back(code);
        ++it;
    }
    if (codes.empty() || it == end || (it + 1) == end || *(it + 1) != ";")
        throw(ErrorServerBlock("error_page needs at least one code, a file and a semi-colon"));
    for (size_t i = 0; i < codes.size(); i++)
        server.error_files[codes[i]] = *it;
    it = it + 2;
}

//on verifie que le fichier demande existe sur le fichier en regardant si c'est un fichier avec un 
//path absolu ou relatif puis en utilisant la fonction stat
//la commande n'est executee que a la fin d'un premier parsing
void    Server::resolve_error_pages()
{
    std::map<int, std::string>::iterator it = server.error_files.begin();

    for (; it != server.error_files.end(); ++it)
    {
        struct stat info;
        std::string path = it->second;

        if (path[0] == '/' && stat(path.c_str(), &info) == 0 && S_ISREG(info.st_mode))
            continue;
        path = server.server_repo + (path[0] == '/' ? "" : "/") + path;
        if (stat(path.c_str(), &info) == -1 || !S_ISREG(info.st_mode))
            throw(ErrorServerBlock("error_page: file not found: " + it->second));
        it->second = path;
    }
}

//on rajoute la root assoicee au fichier demande
void    Server::resolve_location_paths()
{
    std::vector<t_location>::iterator it = server.location_blocks.begin();

    for (; it != server.location_blocks.end(); ++it)
    {
        struct stat info;

        if (!it->file_directory.empty() && it->file_directory[0] != '/')
            it->file_directory = server.server_repo + "/" + it->file_directory;
        if (it->upload_dir.empty())
            continue;
        if (it->upload_dir[0] != '/')
            it->upload_dir = server.server_repo + "/" + it->upload_dir;
        if (stat(it->upload_dir.c_str(), &info) == -1 || !S_ISDIR(info.st_mode))
            throw(ErrorServerBlock("upload_store: directory not found: " + it->upload_dir));
    }
}

void    Server::client_size(IT &it, IT end)
{
    if ((it + 1) == end || *(it + 1) != ";")
        throw(ErrorServerBlock("client_max_body_size requires one argument and a semi-colon"));
    transform_to_num(*it, server.max_body_size);
    it = it + 2;
}

//on verifie que :PORT est impossible en acceptant PORT
void    Server::listen(IT &it, IT end)
{
    std::string host;
    std::string port_str = *it;
    size_t colon;
    int port;

    if ((it + 1) == end || *(it + 1) != ";")
        throw(ErrorServerBlock("listen requires one argument and a semi-colon"));
    colon = port_str.rfind(':');
    if (colon != std::string::npos)
    {
        host = port_str.substr(0, colon);
        port_str = port_str.substr(colon + 1);
        if (host.empty())
            throw(ErrorServerBlock("listen: empty interface before ':'"));
    }
    if (port_str.empty() || port_str.size() > 5
        || port_str.find_first_not_of("0123456789") != std::string::npos)
        throw(ErrorServerBlock("listen: invalid port " + *it));
    transform_to_num(port_str, port);
    if (port < 1 || port > 65535)
        throw(ErrorServerBlock("listen: port must be between 1 and 65535"));
    server.open_ports.insert(port);
    server.listen_addrs.push_back(std::make_pair(host, port));
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