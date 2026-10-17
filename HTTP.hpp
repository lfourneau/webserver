#ifndef HTTP_HPP
# define HTTP_HPP

# include <iostream>
# include <sstream>
# include <unistd.h>
# include <string>
# include <map>
# include "HelperFunctions.hpp"

class HTTP
{
    public:
    HTTP(int fd);
    HTTP(const HTTP &other);
    HTTP &operator=(const HTTP &other);
    ~HTTP();
    void    append_content(const char *content, unsigned int total_read);
    void    parse_input();
    void    parse_initial_response_line();
    void    parse_header_lines();
    int     get_client_fd();
    void    print_content();
    private:
    int     associated_fd;
    std::string request_content;
    std::string request_method;
    std::string URI;
    std::string protocol_version;
    void    analyse_buffer();
    std::map<std::string, std::string> header_lines;

};

#endif