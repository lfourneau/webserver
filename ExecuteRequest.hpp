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
    int code;
    s_execute() : output(), filename(), code(200) {};
} t_execute;

class ExecuteRequest
{
    public:
    ExecuteRequest(t_server const &serv, t_location const &loc, t_header const &head);
    ExecuteRequest(ExecuteRequest const &other);
    ExecuteRequest &operator=(ExecuteRequest const &other);
    ~ExecuteRequest();
    void execute();
    int get_code() const;
    std::string const &get_output() const;
    std::string const &get_filename() const;

    private:
    ExecuteRequest();
    t_server const *serv;
    t_location const *loc;
    t_header const *head;
    t_execute exec_info;
    int find_method() const;
    void build_path();
    void complete_repo();
    void complete_file();
};

#endif
