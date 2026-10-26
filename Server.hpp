#ifndef SERVER_HPP
# define SERVER_HPP

# include <vector>
# include <string>
# include <map>
# include <exception>
# include "Location.hpp"

class Server
{
    public:
    Server(std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end);
    Server(Server const &other);
    Server &operator=(Server const &other);
    ~Server();
    class InvalidTokenAfterServer : public std::exception
    {
        public:
        virtual const char * what() const throw()
        {
            return ("Invalid token after server");
        }
    };
    private:
    std::vector<int> open_ports;
    std::vector<std::string> server_names;
    std::map<std::string, std::string> error_pages;
    int max_body_size;
    std::vector<Location> location_blocks;

    void    Server::retrieve_info(std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end);
};

#endif