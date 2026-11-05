#ifndef EXECUTEREQUEST_HPP
# define EXECUTEREQUEST_HPP

# include <string>
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# include "Server.hpp"
# include "Location.hpp"
# include "HTTP.hpp"
# define AUTOINDEX "autoindex.py"

typedef struct s_execute
{
    std::string output;
    std::string filename;
} t_execute;

class ExecuteRequest
{
    public:
    ExecuteRequest();
    ExecuteRequest(ExecuteRequest const &other);
    ExecuteRequest &operator=(ExecuteRequest const &other);
    ~ExecuteRequest();
    private:
    std::string const get_content(t_server const &serv, t_location const *loc, t_header * head);
    void complete_file_path(t_server const &serv, t_location const *loc, t_header * head);
    void complete_repo(t_server const &serv, t_location const *loc, t_header * head);
    void complete_file(t_server const &serv, t_location const *loc, t_header * head);
    std::string get_output(t_server const &serv, t_location const *loc, t_header *head);
    t_execute exec_info;
};

#endif