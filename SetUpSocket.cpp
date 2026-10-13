# include "SetUpSocket.hpp"

SetUpSocket::SetUpSocket(int domain, int type, int protocol, std::string port) : domain(domain), \
type(type), protocol(protocol), port(port)
{
    std::memset(&this->parameter, 0, sizeof(this->parameter));
    parameter.ai_family = domain;
    parameter.ai_socktype = type;
    parameter.ai_flags = AI_PASSIVE;

    try
    {
        get_addr();
        socket_creation();
        bind_socket();
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
    return (*this);
}

SetUpSocket::~SetUpSocket()
{
    ;
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
    if (bind(fd_socket, return_getaddr_info->ai_addr, sizeof(*return_getaddr_info->ai_addr)))
    {
        throw(BindingFail());
    }
}