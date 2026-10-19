#ifndef CGI_HPP
# define CGI_HPP

# include <vector>
# include <utility>
# include <string>
# include "HTTP.hpp"
# include <unistd.h>
# include <fcntl.h>
# include <sys/wait.h>
# include <string.h>

class   CGI
{
    public:
        CGI(HTTP &other);
        CGI(CGI const &other);
        CGI &operator=(CGI const &other);
        ~CGI();
        std::string get_output();
    private:
        std::string    exec_program;
        char    **argv;
        std::vector<char *> envp;
        std::vector<std::pair <std::string, std::string> > const env;
        std::string const method;
        std::string const file;
        std::string const protocol;
        std::string host;
        std::string const fd;
        std::string const content;
        std::string output_cgi;
        void    fork_program();
        void    parse_input();
        void    basic_envp();
        void    script_name();
        void    extract_host_port();
        void    sup_post();
        std::string const find_type() const;
};

# include "find_structure.tpp"

#endif