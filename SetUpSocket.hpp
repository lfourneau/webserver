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
        std::map<int, int> fds_listen;
        std::vector<struct addrinfo*> structures_addr_info;
        const int domain;
        const int type;
        const int protocol;
        const std::string port;
        struct addrinfo parameter;
        struct addrinfo *return_getaddr_info;
        ServerConfig *config;

        void    for_each_block();
        void    socket_creation(struct addrinfo *structure, int i);
        void    get_addr(t_server const &other);
        void    bind_socket(int &file_desc, struct addrinfo *info);
        void    listen_socket(int &file_desc);
        void    set_connections_fd_set(fd_set &keep);
        void    monitor_connections();
        void    set_sock_non_crash(int &file_descriptor);
        bool    check_port_not_already_used(int port);

};

# include "find_structure.tpp"

#endif