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
# include <set>
# include "HTTP.hpp"
# include "ServerConfig.hpp"

class SetUpSocket
{
    public:
        SetUpSocket(ServerConfig *config);
        SetUpSocket(SetUpSocket const &other);
        SetUpSocket &operator=(SetUpSocket const &other);
        ~SetUpSocket();
        class SocketSetUpFail : public std::exception
        {
            public:
            SocketSetUpFail(std::string reason) : res(reason) {};
            ~SocketSetUpFail() {};
            virtual const char *what() const throw()
            {
                return "socket creation has failed";
            }
            private:
            std::string const res;
        };
        private:
        std::set<int> fds_listen;
        std::vector<struct addrinfo*> structures_addr_info;
        const int domain;
        const int type;
        const int protocol;
        const std::string port;
        struct addrinfo parameter;
        struct addrinfo *return_getaddr_info;
        ServerConfig *config;

        void    socket_creation();
        void    get_addr();
        void    bind_socket();
        void    listen_socket();
        void    monitor_connections();
        void    set_sock_non_crash();
};

# include "find_structure.tpp"

#endif