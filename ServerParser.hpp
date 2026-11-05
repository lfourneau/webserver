#ifndef SERVERPARSER_HPP
# define SERVERPARSER_HPP

# include "HTTP.hpp"
# include "sys/stat.h"
# include "ErrorCodes.hpp"

class ServerParser
{
    public:
    static t_header *parse_input(std::string const &other, ssize_t const size);
    static void get_first_line(std::stringstream &other, t_header *save);
    static void check_cgi(t_header *head);
    static void parse_headers(std::stringstream &val, t_header* header);
    static void get_host_port(t_header *header);
    private:
    ServerParser();
    ServerParser(ServerParser const &other);
    ServerParser &operator=(ServerParser const &other);
    ~ServerParser();
};

# include "transform_num.tpp"

#endif