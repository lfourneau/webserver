#ifndef SETUPSOCKET_HPP
# define SETUPSOCKET_HPP

# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <netinet/ip.h>
# include <string.h>
# include <netdb.h>
# include <iostream>

class SetUpSocket
{
    public:
        SetUpSocket();
        SetUpSocket(SetUpSocket const &other);
        SetUpSocket operator=(SetUpSocket const &other);
        ~SetUpSocket();
    private:
        int fd;
        struct addrinfo parameter;
        struct addrinfo return_getaddr_info;

}

#endif