#include "HTTP.hpp"
#include "SetUpSocket.hpp"
#include "ErrorCodes.hpp"
#include "ServerConfig.hpp"

/*
Steps in the creation of a webserver
1) Create a socker
2) Bind it to an IP port
3) Mark the socket for listening in
4) Accpet a call
5) Close the listening socket
6) While receiving the message echo its content
7) Close  the socket*/

int main(int ac, char **av)
{
    ServerConfig *configurations = NULL;
    SetUpSocket *server_exec = NULL;

    if (ac != 2)
    {
        std::cerr << "Invalid amount of arguments" << std::endl;
        return 1; 
    }

    std::string value(av[1]);

    try
    {
        configurations = new ServerConfig(value);
        server_exec = new SetUpSocket(configurations);
    }
    catch (std::exception &e)
    {
        if (configurations != NULL)
            delete configurations;
        if (server_exec != NULL)
            delete server_exec;
        std::cerr << "Error: " << e.what() << std::endl;
        return (2);
    }
    return (0);
}