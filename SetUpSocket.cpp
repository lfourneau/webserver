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
        get_addr();
        socket_creation();
        set_sock_non_crash();
        bind_socket();
        listen_socket();
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
        freeaddrinfo(this->return_getaddr_info);
}

void    SetUpSocket::get_addr()
{
    std::vector<Server>::const_iterator it;
    std::vector<Server>::const_iterator end;
    struct addrinfo new_addrinfo;

    for (it = this->config->get_server().begin(); it != end; it++)
    {
        while (*it)
        if (getaddrinfo(NULL, port.c_str(), &this->parameter, &this->return_getaddr_info))
            throw(SocketSetUpFail("getaddrinfo has failed"));
        
    }
}

void    SetUpSocket::socket_creation()
{
    int new_listen_fd;

    new_listen_fd = socket(this->domain, this->type, 0);
    if (new_listen_fd == -1)
        throw (SocketSetUpFail("socket creation has failed"));
    fds_listen.insert(new_listen_fd);
}

void    SetUpSocket::set_sock_non_crash()
{
    int option = true;

    if (setsockopt(fd_socket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) < 0)
    {
        close(fd_socket);
        throw(SocketSetUpFail("sockoption has failed"));
    }
}

void    SetUpSocket::bind_socket()
{
    if (bind(fd_socket, return_getaddr_info->ai_addr, return_getaddr_info->ai_addrlen))
        throw(SocketSetUpFail("binding has failed"));
}

void    SetUpSocket::listen_socket()
{
    //somaxconn defines the maximum concurrent connections the server
    //can receive
    if (listen(fd_socket, SOMAXCONN))
        throw(SocketSetUpFail("listening has failed"));
}

void    SetUpSocket::monitor_connections()
{
    fd_set keep, read;
    int     return_select; 
    int     fd_received;
    char    buffer[4096];
    std::vector<HTTP>   list_files = {};

    FD_ZERO(&keep);
    FD_SET(fd_socket, &keep);

    while (1)
    {
        read = keep;
        return_select = select(FD_SETSIZE, &read, NULL, NULL, NULL);
        if (return_select)
        { 
            if (return_select < 0)
            {
                throw(SocketSetUpFail("select has failed"));
            }
            else
            {
                for (int i = 0; i < FD_SETSIZE; i++)
                {
                    if (!FD_ISSET(i, &read))
                        continue;
                    if (i == fd_socket)
                    {
                        fd_received = accept(fd_socket, NULL, NULL);
                        FD_SET(fd_received, &keep);

                        HTTP    new_connection(fd_received);
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
                            HTTP    &structure = find_value_vector(list_files, &HTTP::get_client_fd, i);

                            structure.analyse_buffer();
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
