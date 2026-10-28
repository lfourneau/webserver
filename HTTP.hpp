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
# include <map>
# include "ServerConfig.hpp"

# define PATH_FILES_DIRECTORY "/home/lfournea/Documents/webserver"

class HTTP
{
    public:
    HTTP(int fd, t_serverconfig const &conf);
    HTTP(const HTTP &other);
    HTTP &operator=(const HTTP &other);
    ~HTTP();
    void    analyse_buffer();
    int    append_content(const char *content, unsigned int total_read);
    void get_value_host();
    int     get_client_fd();
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
    int len_body;
    bool    cgi_request;
    int code;
    std::string error_message;
    bool headers_parsed;
    t_serverconfig const &server_rules;
    bool bloc_found;
    t_server bloc;
    std::map<std::string, std::string> header_lines;
    
    //helper functions
    void    print_content();

    //parse the input
    void    parse_initial_response_line(std::stringstream &flux);
    void    parse_header_lines(std::stringstream &flux);
    void    add_to_previous_pair(std::string &content);
    void    http_body(std::stringstream &flux);
    void    determine_extension();
    void    perform_request();
    bool    all_body_received();
    void    find_bloc_request();
    bool    find_location_block(t_location &find);

    //GET content methods
    void    read_file();
    void    read_and_store_file(int fd);

    //CGI requests
    bool    check_cgi_request();

    //send to browser methods
    void    cgi_content(void);

    //build error
    void    build_error(int error_code, std::string message);

    //send response
    void    encapsulate_response();

    //check authorizations methods
    bool    check_authorized_method(std::string const &val);
};

# include "find_structure.tpp"
# include "concatenate.tpp"

#endif