# include "SetUpSocket.hpp"
# include "ServerConfig.hpp"
# include "HTTP.hpp"

SetUpSocket::SetUpSocket(ServerConfig *config) : domain(AF_INET), \
type(SOCK_STREAM), protocol(0), return_getaddr_info(NULL), config(config)
{
    std::memset(&this->parameter, 0, sizeof(this->parameter));
    parameter.ai_family = domain;
    parameter.ai_socktype = type;
    parameter.ai_flags = AI_PASSIVE;

    try
    {
        for_each_block();
        monitor_connections();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
}

SetUpSocket::SetUpSocket(SetUpSocket const &other) : domain(other.domain), \
type(other.type), protocol(other.protocol), port(other.port)
{
    ;
}

SetUpSocket &SetUpSocket::operator=(SetUpSocket const &other)
{
    (void)other;
    return (*this);
}

SetUpSocket::~SetUpSocket()
{
    if (this->return_getaddr_info != NULL)
    {
        std::vector<struct addrinfo *>::iterator it = structures_addr_info.begin();

        for (; it != structures_addr_info.end(); ++it)
        {
            freeaddrinfo(this->return_getaddr_info);
        }
    }
}

void    SetUpSocket::for_each_block()
{
    std::vector<t_server>::const_iterator it;

    for (it = this->config->get_server_config().servers.begin(); \
    it != this->config->get_server_config().servers.end() ; it++)
    {
        get_addr(*it);
    }
}

void    SetUpSocket::get_addr(t_server const &serv)
{
    std::set<int>::const_iterator it;
    std::set<int>::const_iterator end = serv.open_ports.end();

    for (it = serv.open_ports.begin(); it != end; it++)
    {
        check_port_not_already_used(*it);
        struct addrinfo *new_addrinfo = NULL;
        std::ostringstream to_string;
        
        if (check_port_not_already_used(*it))
            continue;
        to_string << *it;
        if (getaddrinfo(NULL, to_string.str().c_str(), &this->parameter,\
         &new_addrinfo))
            throw(SocketSetUpFail("getaddrinfo has failed"));
        structures_addr_info.push_back(new_addrinfo);

        struct addrinfo *next = new_addrinfo->ai_next;
        while (true)
        {
            socket_creation(new_addrinfo, *it);
            if (next == NULL)
                break;
            new_addrinfo = next;
            next = new_addrinfo->ai_next;
        }
    }
}

bool    SetUpSocket::check_port_not_already_used(int port)
{
    std::map<int, int>::iterator it = fds_listen.begin();

    for (; it != fds_listen.end(); ++it)
    {
        if (it->second == port)
            return (true);
    }
    return (false);
}

void    SetUpSocket::socket_creation(struct addrinfo *info, int i)
{
    int new_listen_fd;

    new_listen_fd = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
    if (new_listen_fd == -1)
        throw (SocketSetUpFail("socket creation has failed"));
    set_sock_non_crash(new_listen_fd);
    bind_socket(new_listen_fd, info);
    listen_socket(new_listen_fd);
    fds_listen[new_listen_fd] = i;
}

void    SetUpSocket::set_sock_non_crash(int &file_desc)
{
    int option = true;

    if (setsockopt(file_desc, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) < 0)
    {
        close(file_desc);
        throw(SocketSetUpFail("sockoption has failed"));
    }
}

void    SetUpSocket::bind_socket(int &file_desc, struct addrinfo *info)
{
    if (bind(file_desc, info->ai_addr, info->ai_addrlen))
        throw(SocketSetUpFail("binding has failed"));
}

void    SetUpSocket::listen_socket(int &file_desc)
{
    //somaxconn defines the maximum concurrent connections the server
    //can receive
    if (listen(file_desc, SOMAXCONN))
        throw(SocketSetUpFail("listening has failed"));
}

void    SetUpSocket::set_connections_fd_set(fd_set &keep)
{
    std::map<int, int>::const_iterator it = fds_listen.begin();

    for (; it != fds_listen.end(); ++it)
    {
        FD_SET(it->first, &keep);
    }
}

void    SetUpSocket::monitor_connections()
{
    fd_set keep, read;
    int     return_select; 
    int     fd_received;
    char    buffer[4096];
    std::vector<HTTP>   list_files;

    FD_ZERO(&keep);
    set_connections_fd_set(keep);

    while (1)
    {
        read = keep;
        return_select = select(FD_SETSIZE, &read, NULL, NULL, NULL);
        if (return_select)
        { 
            if (return_select < 0)
                throw(SocketSetUpFail("select has failed"));
            else
            {
                for (int i = 0; i < FD_SETSIZE; i++)
                {
                    if (!FD_ISSET(i, &read))
                        continue;
                    if (fds_listen.count(i))
                    {
                        fd_received = accept(i, NULL, NULL);
                        FD_SET(fd_received, &keep);

                        HTTP    new_connection(fd_received, this->config->get_server_config());
                        list_files.push_back(new_connection);
                    }
                    else
                    {
                        std::memset(buffer, 0, 4096);
                        int total_read;
                        total_read = recv(i, buffer, sizeof(buffer) - 1, 0);
                        if (total_read == -1)
                            throw(SocketSetUpFail("recv() has failed"));
                        if (total_read == 0)
                        {
                            delete_vector(list_files, &HTTP::get_client_fd, i);
                            FD_CLR(i, &keep); 
                        }
                        else
                        {
                            HTTP    &structure = find_value_vector(list_files, &HTTP::get_client_fd, i);

                            if (structure.append_content(buffer, total_read))
                            {
                                delete_vector(list_files, &HTTP::get_client_fd, i);
                                FD_CLR(i, &keep); 
                            }
                        }
                    }
                }
            }
        }
    }
}
