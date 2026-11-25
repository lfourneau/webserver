#ifndef FINDSERVER_HPP
# define FINDSERVER_HPP

# include "Server.hpp"
# include "HTTP.hpp"

class FindServer
{
    public:
    static t_server const &find_server(t_header const *val, std::vector<t_server> const &servers);
    static t_location const &find_location(t_header const *val, t_server const &servers);
    static t_location const *find_cgi(t_header const *val, t_location const *);
    static t_location const *find_repo(t_header const *val, t_location const *loc, unsigned int &len);
    private:
    FindServer();
    FindServer(FindServer const &other);
    FindServer &operator=(FindServer const &other);
    ~FindServer();
};

#endif