#ifndef SERVERCONFIG_HPP
# define SERVERCONFIG_HPP

# include <string>
# include <fcntl.h>
# include <exception>
# include <unistd.h>
# include <sstream>
# include <iostream>
# include <vector>
# include "Server.hpp"
# include "Location.hpp"

typedef struct s_serverconfig
{
    std::vector<std::string> tokens;
    std::vector<t_server> servers;
} t_serverconfig;

class ServerConfig
{
    public:
    ServerConfig(std::string &path_config);
    ServerConfig(ServerConfig const &other);
    ServerConfig &operator=(ServerConfig const &other);
    ~ServerConfig(); 
    t_serverconfig const &get_server_config() const;

    class FileOpenFail : public std::exception
    {
        virtual const char *what() const throw()
        {
            return ("Configuration file could not be opened");
        }
    };
    class ReadFail : public std::exception
    {
        virtual const char *what() const throw()
        {
            return ("Read failed");
        }
    };
    class BracketsNotClosed : public std::exception
    {
        virtual const char *what() const throw()
        {
            return ("Missing bracket");
        }
    };

    private:
    t_serverconfig servconfig;

    void open_read_config(std::string &content, std::string &config);
    void parse_content(std::stringstream &content);
    void parse_server_block(std::stringstream &flux, int open_brackets, int closed_brackets);
    void tokenize(std::string const &content);
    void reinitialize_string(std::string &content);
    void recup_information();
};

#endif