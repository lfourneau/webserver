#ifndef SERVER_HPP
# define SERVER_HPP

# include <vector>
# include <string>
# include <map>
# include <set>
# include <utility>
# include <exception>
# include <cstring>
# include "Location.hpp"

typedef struct s_server
{
    std::string server_repo;
    std::set<int> open_ports;
    std::vector<std::pair<std::string, int> > listen_addrs;
    std::vector<std::string> server_names;
    std::map<int, std::string> error_files;
    int max_body_size;
    std::vector<t_location> location_blocks;
    s_server() : server_repo(), open_ports(), listen_addrs(), server_names(), \
    error_files(), max_body_size(0), location_blocks() {}; 
} t_server;

class Server
{
    public:
    typedef std::vector<std::string>::iterator IT;
    Server(IT &it, IT end);
    Server(Server const &other);
    Server &operator=(Server const &other);
    virtual ~Server() throw();
    class ErrorServerBlock : public std::exception
    {
        public:
        ErrorServerBlock(std::string content) : val(content) {};
        virtual ~ErrorServerBlock() throw() {};
        virtual const char * what() const throw()
        {
            return (val.c_str());
        }
        private:
        std::string val;
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
    void    resolve_error_pages();
    void    resolve_location_paths();
};

# include "transform_num.tpp"

#endif