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

enum e_status
{
    PARSE_REQUEST,
    PARSE_HEADER,
    PARSE_BODY,
    APPLY_RULES,
};

class HTTP
{
    public:
    HTTP(int fd, std::vector<t_server> const &conf);
    HTTP(const HTTP &other);
    HTTP &operator=(const HTTP &other);
    virtual ~HTTP() throw();
    void    analyse_buffer();
    int    append_content(const char *content, unsigned int total_read);
    void get_value_host();
    int     get_client_fd();

    private:
    std::vector<t_server> const &server_rules;
    std::string request_content;
    bool    cgi_request;
    e_status status;
    int code;
    std::string error_message;

    protected:
    int     associated_fd;
    std::string content;
    //on garde
    std::string body;

    t_server bloc;
    t_location loc;
    std::map<std::string, std::string> info_request;
    
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
    
    //implement file rerouting
    void    modify_route(std::string &ref);

    //check path doesn't lead to file
    bool    contains_a_file();
};

# include "find_structure.tpp"
# include "concatenate.tpp"
# include "transform_num.tpp"

#endif