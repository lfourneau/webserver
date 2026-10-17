# include "SetUpSocket.hpp"
# include "HTTP.hpp"

SetUpSocket::SetUpSocket(int domain, int type, int protocol, std::string port) : domain(domain), \
type(type), protocol(protocol), port(port), return_getaddr_info(NULL)
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

void    SetUpSocket::socket_creation()
{
    fd_socket = socket(this->domain, this->type, 0);
    if (fd_socket == -1)
        throw (SocketCreationFailure());
}

void    SetUpSocket::get_addr()
{
    if (getaddrinfo(NULL, port.c_str(), &this->parameter, &this->return_getaddr_info))
    {
        throw(GetAddrInfoFail());
    }
}

void    SetUpSocket::bind_socket()
{
    if (bind(fd_socket, return_getaddr_info->ai_addr, return_getaddr_info->ai_addrlen))
    {
        throw(BindingFail());
    }
}

void    SetUpSocket::listen_socket()
{
    //somaxconn defines the maximum concurrent connections the server
    //can receive
    if (listen(fd_socket, SOMAXCONN))
        throw(ListeningFailed());
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
                throw(SelectFailed());
            }
            else
            {
                for (int i = 0; i < FD_SETSIZE; i++)
                {
                    if (!FD_ISSET(i, &read))
                        continue;
                    if (i == fd_socket)
                    {
                        std::cout << "new connection received" << std::endl;
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
                            throw(RecvException());
                        if (total_read == 0)
                        {
                            HTTP    &structure = find_value_vector(list_files, &HTTP::get_client_fd, i);

                            structure.analyse_buffer();
                            FD_CLR(i, &keep); 
                        }
                        else
                        {
                            HTTP    &structure = find_value_vector(list_files, &HTTP::get_client_fd, i);
                            if (structure.append_content(buffer, total_read))
                            {
                                FD_CLR(i, &keep); 
                            }
                        }
                    }
                }
            }
        }
    }
}

void    SetUpSocket::set_sock_non_crash()
{
    int option = true;

    if (setsockopt(fd_socket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) < 0)
    {
        close(fd_socket);
        throw(SockOptFail());
    }
}