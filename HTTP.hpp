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

# define PATH_FILES_DIRECTORY "/home/lfournea/Documents/webserver/html_files"

class HTTP
{
    public:
    HTTP(int fd);
    HTTP(const HTTP &other);
    HTTP &operator=(const HTTP &other);
    ~HTTP();
    void    analyse_buffer();
    int     get_client_fd();
    int    append_content(const char *content, unsigned int total_read);
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
    private:
    int     associated_fd;
    std::string request_content;
    std::string request_method;
    std::string URI;
    std::string protocol_version;
    std::string pwd;
    int code;
    void    parse_input();
    void    parse_initial_response_line();
    void    parse_header_lines();
    void    print_content();
    void    send_to_client();
    void    send_response();
    void    send_response(const char *error);
    void    read_and_store_file(int fd);
    void    GETResponse(std::string const &other, size_t size_file);
    std::string const   output_first_line();
    std::string const   content_length(size_t &value) const;
    std::string const   determine_extension(std::string const &other) const;
    std::vector<std::pair<std::string, std::string> > header_lines;
};

#endif