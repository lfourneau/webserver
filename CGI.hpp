#ifndef CGI_HPP
# define CGI_HPP

# include <vector>
# include <utility>
# include <string>
# include "HTTP.hpp"

class   CGI
{
    public:
        CGI(HTTP &other);
        CGI(CGI const &other);
        CGI &operator=(CGI const &other);
        ~CGI();
    private:
        char    *exec_program;
        char    **argv;
        std::vector<std::string> envp;
        std::vector<std::pair <std::string, std::string> > const env;
        std::string const method;
        std::string const file;
        std::string const protocol;
        std::string host;
        std::string const fd;
        void    fork_program();
        void    parse_input();
        void    basic_envp();
        void    script_name();
        void    extract_host_port();
};

#endif