#ifndef SETUPSOCKET_HPP
# define SETUPSOCKET_HPP

# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <netinet/ip.h>
# include <string.h>
# include <string>
# include <netdb.h>
# include <iostream>
# include <exception>
# include <sstream>
# include <cstring>
# include <unistd.h>
# include <vector>
# include "HTTP.hpp"

class SetUpSocket
{
    public:
        SetUpSocket(int domain, int type, int protocol, std::string port);
        SetUpSocket(SetUpSocket const &other);
        SetUpSocket &operator=(SetUpSocket const &other);
        ~SetUpSocket();
        class SocketCreationFailure : public std::exception
        {
            virtual const char *what() const throw()
            {
                return "socket creation has failed";
            }
        };
        class GetAddrInfoFail : public std::exception
        {
            virtual const char *what() const throw()
            {
                return "getaddrinfo has failed";
            }
        };
        class BindingFail : public std::exception
        {
            virtual const char *what() const throw()
            {
                return "binding has failed";
            }
        };
        class ListeningFailed : public std::exception
        {
            virtual const char *what() const throw()
            {
                return "listening has failed";
            }
        };       
        class SelectFailed : public std::exception
        {
            virtual const char *what() const throw()
            {
                return "select has failed";
            }
        };           
        class SockOptFail : public std::exception
        {
            virtual const char *what() const throw()
            {
                return "sockoption has failed";
            }
        };  
        class RecvException : public std::exception
        {
            virtual const char *what() const throw()
            {
                return "recv() has failed";
            }
        };  
        private:
        int fd_socket;
        const int domain;
        const int type;
        const int protocol;
        const std::string port;
        struct addrinfo parameter;
        struct addrinfo *return_getaddr_info;
        void    socket_creation();
        void    get_addr();
        void    bind_socket();
        void    listen_socket();
        void    monitor_connections();
        void    set_sock_non_crash();
};

# include "find_structure.tpp"

#endif