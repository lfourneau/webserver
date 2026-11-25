#ifndef ENCAPSULATE_HPP
# define ENCAPSULATE_HPP

# include <string>
# include <map>
# include "Server.hpp"
# include "ErrorCodes.hpp"

# define SERVER_NAME "webserv/1.0"

class Encapsulate
{
    public:
    static std::string response(int code, std::string const &body, \
        std::string const &filename, bool keep_alive = true);
    static std::string cgi_response(int code, std::string const &cgi_output, \
        bool keep_alive = true);
    static std::string error(int code, std::string const &body, \
        bool keep_alive = false, std::string const &allow = "");
    static std::string redirect(int code, std::string const &location);

    static std::string content_type(std::string const &filename);
    static std::string error_body_from_cgi(std::string const &cgi_output);
    static bool read_error_file(std::string const &path, std::string &out);
    static std::string default_error_page(int code);

    private:
    Encapsulate();
    Encapsulate(Encapsulate const &other);
    Encapsulate &operator=(Encapsulate const &other);
    ~Encapsulate();

    typedef std::map<std::string, std::string> t_headers;

    static std::string status_line(int code);
    static std::string assemble(int code, t_headers headers, std::string const &body);
    static void split_cgi_output(std::string const &cgi_output, t_headers &headers, \
        std::string &body, int &code);
    static std::string current_date();
};

#endif
