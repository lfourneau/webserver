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
# include <cstring>


class   CGI : public HTTP
{
    public:
        CGI(HTTP const &other);
        CGI(CGI const &other);
        CGI &operator=(CGI const &other);
        ~CGI();
        std::string get_output();
    private:
    
        std::string    exec_program;
        std::vector<char *> argv;
        std::vector<char *> envp;
        std::string output_cgi;

        void    fork_program();
        void    parse_input();
        void    basic_envp(std::vector<std::string> &vec);
        void    script_name(std::vector<std::string> &vec);
        void    sup_post(std::vector<std::string> &vec);
        void    transform_to_c_str(std::vector<std::string> const &vec);
        void    create_argv();
        void    free_char_lists();
        std::string const find_type() const;
};

# include "find_structure.tpp"
# include "concatenate.tpp"

#endif