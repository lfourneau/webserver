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

class ServerConfig
{
    public:
    ServerConfig(std::string &path_config);
    ServerConfig(ServerConfig const &other);
    ServerConfig &operator=(ServerConfig const &other);
    ~ServerConfig(); 
    std::vector<t_server> const &get_server_config() const;

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
    class WhiteChar : public std::exception
    {
        virtual const char *what() const throw()
        {
            return ("probable wrong character end file");
        }
    };
    protected:
    std::vector<t_server> servers;

    private:
    std::vector<std::string> tokens;

    void open_read_config(std::string &content, std::string &config);
    void parse_content(std::stringstream &content);
    void parse_server_block(std::stringstream &flux, int open_brackets, int closed_brackets);
    void tokenize(std::string const &content);
    void reinitialize_string(std::string &content);
    void recup_information();
};

#endif