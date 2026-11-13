#ifndef SETUPSOCKET_HPP
# define SETUPSOCKET_HPP

# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <netinet/ip.h>
# include <string>
# include <netdb.h>
# include <iostream>
# include <exception>
# include <sstream>
# include <cstring>
# include <unistd.h>
# include <vector>
# include <set>
# include <map>
# include <poll.h>
# include <csignal>

extern volatile sig_atomic_t g_stop;
# include "HTTP.hpp"
# include "ServerConfig.hpp"

class SetUpSocket
{
    public:
        SetUpSocket(ServerConfig *config);
        SetUpSocket(SetUpSocket const &other);
        SetUpSocket &operator=(SetUpSocket const &other);
        virtual ~SetUpSocket() throw();
        class SocketSetUpFail : public std::exception
        {
            public:
            SocketSetUpFail(std::string reason) : res(reason) {};
            virtual ~SocketSetUpFail() throw() {};
            virtual const char *what() const throw()
            {
                return (res.c_str());
            }
            private:
            std::string res;
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
        // "127.0.0.1:8080" -> fd du socket d'ecoute
        std::map<std::string, int> bound_addrs;
        // fd du socket d'ecoute -> blocs server qui l'utilisent
        std::map<int, std::vector<t_server> > listen_servers;
        int     socket_creation(int family, struct sockaddr *addr, socklen_t addr_len, \
            int port, std::string const &name);
        int     wildcard_socket(int port, std::string const &name);
        void    check_virtual_hosts();
        void    get_addr(t_server const &other);
        // a quoi correspond chaque case du tableau donne a poll()
        enum e_fd_type { FD_LISTEN, FD_CLIENT, FD_CGI_PIPE };
        typedef struct s_poll_owner
        {
            e_fd_type type;
            int client_fd;
            HTTP *conn;
            unsigned int generation;
        } t_poll_owner;

        std::map<int, HTTP *> clients;

        void    monitor_connections();
        int     build_poll_list(std::vector<struct pollfd> &pfds, \
            std::vector<t_poll_owner> &owners);
        void    add_poll_entry(std::vector<struct pollfd> &pfds, \
            std::vector<t_poll_owner> &owners, int fd, short events, \
            e_fd_type type, int client_fd, HTTP *conn, unsigned int generation);
        void    dispatch(struct pollfd const &pfd, t_poll_owner const &owner, \
            std::set<int> &touched);
        void    accept_client(int listen_fd, std::set<int> &touched);
        void    close_client(int fd, std::set<int> &touched);
        void    check_timeouts(std::set<int> &touched);
        void    set_non_blocking(int fd);
        void    set_sock_non_crash(int &file_descriptor);

};

# include "find_structure.tpp"

#endif