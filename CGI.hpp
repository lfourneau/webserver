#ifndef CGI_HPP
# define CGI_HPP

# include "HTTP.hpp"
# include "ExecuteRequest.hpp"
# include "Location.hpp"
# include "ErrorCodes.hpp"
# include <string>
# include <algorithm>


typedef struct s_cgi
{
    char *prog;
    char **argv;
    std::vector<std::string> argv_str;
    char **envp;
    std::vector<std::string> content;
    s_cgi() : prog(NULL), argv(NULL), envp(NULL) {}

} t_cgi;

class CGI
{
    public:
    CGI();
    CGI(CGI const &other);
    CGI &operator=(CGI const &other);
    ~CGI();
    static void run(t_execute *exec, t_header const *head, t_location const *loc);
    void extract_output(t_execute *, t_header const *, t_location const *);
    private:
    t_cgi cgi;
    void make_headers(t_execute *, t_header const *, t_location const *);
    void transform_client_headers(t_header const *head);
    void add_additionnal_headers(t_execute *exec, t_header const *head, \
    t_location const *loc);
    void make_char_stars(t_header const *head, t_location const *loc);
    void create_argv(t_header const *, t_location const *);
    void transform_to_c_str(std::vector<std::string> const &vec, char **&dest);
    void fork_program(t_execute *exec, \
        t_header const *head, t_location const *loc);
    void free_char_lists(char **&cont);

};

/*
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
        virtual ~CGI() throw();
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
*/

#endif