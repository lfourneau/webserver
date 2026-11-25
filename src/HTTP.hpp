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
# include <cstring>
# include <sys/socket.h>
# include <cstdio>
# include <cstdlib>
# include <map>
# include <ctime>
# include "ServerConfig.hpp"
# include "ErrorCodes.hpp"


enum e_status
{
    PARSE_REQUEST,
    PARSE_HEADER,
    PARSE_BODY,
    APPLY_RULES,
};

typedef struct s_header
{
    std::string method;
    std::string URI;
    std::string file_name;
    std::string cgi_extension;
    std::string protocol;
    std::string host_name;
    std::string body_request;
    int     host_port;
    bool    file;
    bool    directory;
    std::map<std::string, std::string> headers;

} t_header;

class CGI;

# define CLIENT_TIMEOUT 30
# define MAX_HEADER_SIZE 16384

class HTTP
{
    public:
    enum e_conn_state { READING_REQUEST, RUNNING_CGI, SENDING_RESPONSE };
    enum e_cgi_purpose { CGI_REQUEST, CGI_AUTOINDEX, CGI_ERROR_PAGE };

    HTTP(int fd, std::vector<t_server> const &other);
    ~HTTP();
    int get_client_fd() const;
    short client_events() const;
    int cgi_fd_in() const;
    int cgi_fd_out() const;
    unsigned int get_cgi_generation() const;
    bool needs_fast_tick() const;

    bool on_readable();
    bool on_writable();
    void on_cgi_event(int pipe_fd, short revents);
    bool on_tick(time_t now);

    private:
    HTTP(HTTP const &other);
    HTTP    &operator=(HTTP const &other);

    int fd;
    std::vector<t_server> const *servers;
    std::string http_request;
    t_header *header_http;
    std::string response;
    size_t response_sent;
    e_conn_state state;
    time_t last_activity;
    t_server const *server;
    t_location const *loc;

    CGI *cgi;
    e_cgi_purpose cgi_purpose;
    unsigned int cgi_generation;
    int error_code;
    std::string error_allow;
    std::string autoindex_dir;
    std::string autoindex_uri;

    void    handle_request();
    bool    build_request();
    bool    body_complete();
    void    start_error(ErrorCodes const &err);
    bool    start_cgi(std::string const &prog, std::string const &script, \
        std::vector<std::string> const &env, std::string const &body, \
        e_cgi_purpose purpose);
    void    check_cgi(time_t now);
    void    finish_cgi(int result);
    void    autoindex_fallback();
    void    set_response(std::string const &content);
    std::string allowed_methods() const;
};

# include "find_structure.tpp"
# include "concatenate.tpp"
# include "transform_num.tpp"

#endif