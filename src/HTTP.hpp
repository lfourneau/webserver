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

// sans activite pendant ce temps (en secondes), la connexion est fermee
# define CLIENT_TIMEOUT 30
// taille maximale de la ligne de requete + headers (431 au-dela)
# define MAX_HEADER_SIZE 16384

/*une connexion client. elle ne bloque jamais : chaque methode on_* fait au
plus un recv/send/read/write, et seulement quand poll() a dit que c'etait
possible. etats successifs :
READING_REQUEST  -> poll surveille POLLIN sur le socket
RUNNING_CGI      -> poll surveille les pipes du script, pas le socket
SENDING_RESPONSE -> poll surveille POLLOUT sur le socket*/
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
    // blocs server qui ecoutent sur l'interface:port de cette connexion
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
//class HTTP
//{
//    public:
//    HTTP(int fd, std::vector<t_server> const &conf);
//    HTTP(const HTTP &other);
//    HTTP &operator=(const HTTP &other);
//    virtual ~HTTP() throw();
//    void    analyse_buffer();
//    int    append_content(const char *content, unsigned int total_read);
//    void get_value_host();
//    int     get_client_fd();
//
//    private:
//    std::vector<t_server> const &server_rules;
//    std::string request_content;
//    e_status status;
//    int code;
//    std::string error_message;
//
//    protected:
//    int     associated_fd;
//    std::string content;
//    //on garde
//    std::string body;
//
//    t_server bloc;
//    t_location loc;
//    std::map<std::string, std::string> info_request;
//    
//    //helper functions
//    void    print_content();
//
//    //parse the input
//    void    parse_initial_response_line(std::stringstream &flux);
//    void    parse_header_lines(std::stringstream &flux);
//    void    add_to_previous_pair(std::string &content);
//    void    http_body(std::stringstream &flux);
//    void    determine_extension();
//    void    perform_request();
//    bool    all_body_received();
//    void    find_bloc_request();
//    bool    find_location_block(t_location &find);
//
//    //GET content methods
//    void    read_file();
//    void    read_and_store_file(int fd);
//
//    //CGI requests
//    bool    check_cgi_request();
//
//    //send to browser methods
//    void    cgi_content(void);
//
//    //build error
//    void    build_error(int error_code, std::string message);
//
//    //send response
//    void    encapsulate_response();
//
//    //check authorizations methods
//    bool    check_authorized_method(std::string const &val);
//    
//    //implement file rerouting
//    void    modify_route(std::string &ref);
//    void    update_file_path();
//
//    //check path doesn't lead to file
//    bool    contains_a_file();
//};

# include "find_structure.tpp"
# include "concatenate.tpp"
# include "transform_num.tpp"

#endif