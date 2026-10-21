#ifndef HTTP_HPP
# define HTTP_HPP

# include <iostream>
# include <sstream>
# include <unistd.h>
# include <string>
# include <vector>
# include "HelperFunctions.hpp"
# include <exception>
# include <utility>
# include <sys/stat.h>
# include <fcntl.h>
# include <string.h>
# include <sys/socket.h>
# include <stdio.h>
# include <stdlib.h>

# define PATH_FILES_DIRECTORY "/home/lfournea/Documents/webserver"

class HTTP
{
    public:
    HTTP(int fd);
    HTTP(const HTTP &other);
    HTTP &operator=(const HTTP &other);
    ~HTTP();
    void    analyse_buffer();
    int    append_content(const char *content, unsigned int total_read);
    void get_value_host();
    int     get_client_fd();
    class   ErrorInHeader : public std::exception
    {
        virtual const char *what() const throw()
        {
            return "Error in header sent";
        }
    };
    class   FileNotFound: public std::exception
    {
        virtual const char *what() const throw()
        {
            return "404 Not Found";
        }
    };
    protected:
    int     associated_fd;
    std::string request_content;
    std::string request_method;
    std::string URI;
    std::string protocol_version;
    std::string pwd;
    std::string content;
    std::string host;
    std::string host_port;
    std::string file_type;
    std::string query_string;
    std::string content_type;
    std::string body;
    bool    cgi_request;
    int code;
    std::vector<std::pair<std::string, std::string> > header_lines;
    
    void    parse_initial_response_line(std::stringstream &flux);
    void    parse_header_lines(std::stringstream &flux);
    void    http_body(std::stringstream &flux);
    void    print_content();
    void    read_file();
    void    read_error_file(const char *error);
    void    read_and_store_file(int fd);
    void    GETResponse(std::string const &other);
    void    add_to_previous_pair(std::string &content);
    bool    check_cgi_request();
    std::string const   output_first_line();
    void    determine_extension();
};

# include "find_structure.tpp"
# include "concatenate.tpp"

#endif