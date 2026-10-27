#ifndef SERVER_HPP
# define SERVER_HPP

# include <vector>
# include <string>
# include <map>
# include <exception>
# include <cstring>
# include "Location.hpp"

typedef struct s_server
{
    std::string server_repo;
    std::vector<int> open_ports;
    std::vector<std::string> server_names;
    std::map<int, std::string> error_files;
    int max_body_size;
    std::vector<Location> location_blocks;
    s_server() : server_repo(), open_ports(), server_names(), \
    error_files(), max_body_size(0), location_blocks() {}; 
} t_server;

class Server
{
    public:
    typedef std::vector<std::string>::iterator IT;
    Server(IT &it, IT end);
    Server(Server const &other);
    Server &operator=(Server const &other);
    ~Server();
    class ErrorServerBlock : public std::exception
    {
        public:
        ErrorServerBlock(std::string content) : val(content) {};
        ~ErrorServerBlock() {};
        virtual const char * what() const throw()
        {
            return ("Invalid token after server");
        }
        private:
        std::string const &val;
    };
    t_server const &get_server() const;

    private:
    t_server server;
    void    retrieve_info(IT &it, IT end);
    void    server_name(IT &it, IT end);
    void    root(IT &it, IT end);
    void    error_pages(IT &it, IT end);
    void    client_size(IT &it, IT end);
    void    listen(IT &it, IT end);
    void    location(IT &it, IT end);
};

#endif