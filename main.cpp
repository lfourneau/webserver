#include "HTTP.hpp"
#include "SetUpSocket.hpp"
#include "ErrorCodes.hpp"
#include "ServerConfig.hpp"
#include <csignal>

# define DEFAULT_CONFIG "configuration/default.conf"

// passe a 1 sur Ctrl-C (SIGINT) ou SIGTERM : la boucle poll() s'arrete
// et toute la memoire est liberee avant la fin du programme
volatile sig_atomic_t g_stop = 0;

static void stop_server(int)
{
    g_stop = 1;
}

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

    if (ac > 2)
    {
        std::cerr << "Usage: ./webserv [configuration file]" << std::endl;
        return 1; 
    }

    // sans argument : fichier de configuration par defaut
    std::string value(ac == 2 ? av[1] : DEFAULT_CONFIG);

    // ecrire dans un pipe ou un socket ferme envoie SIGPIPE, qui tuerait
    // le serveur : on l'ignore, write/send renvoient simplement -1
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, stop_server);
    signal(SIGTERM, stop_server);

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
    delete server_exec;
    delete configurations;
    std::cout << "\nwebserv stopped" << std::endl;
    return (0);
}