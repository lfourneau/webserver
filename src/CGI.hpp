#ifndef CGI_HPP
# define CGI_HPP

# include "HTTP.hpp"
# include "ExecuteRequest.hpp"
# include "Location.hpp"
# include "ErrorCodes.hpp"
# include <string>
# include <vector>
# include <ctime>
# include <sys/types.h>

# define PYTHON_INTERPRETER "/usr/bin/python3"
# define PHP_INTERPRETER "/usr/bin/php-cgi"
# define ERROR_SCRIPT "cgi-bin/error_page.py"
# define AUTOINDEX_SCRIPT "cgi-bin/autoindex.py"
# define CGI_TIMEOUT 5

class CGI
{
    public:
    enum e_result { RUNNING, SUCCESS, FAILURE, TIMEOUT };

    CGI();
    ~CGI();

    bool start(std::string const &prog, std::string const &script, \
        std::vector<std::string> const &env, std::string const &body, \
        bool in_script_dir = true);
    void on_writable();
    void on_readable();
    e_result update(time_t now);

    int get_fd_in() const;
    int get_fd_out() const;
    bool waiting_for_exit() const;
    std::string const &get_output() const;

    static std::vector<std::string> request_env(t_execute const *exec, \
        t_header const *head);
    static std::string interpreter_for(std::string const &script);

    private:
    CGI(CGI const &other);
    CGI &operator=(CGI const &other);

    pid_t pid;
    int fd_in;
    int fd_out;
    std::string body;
    size_t body_sent;
    std::string output;
    time_t start_time;
    e_result result;

    void close_fd(int &fd);
    void kill_child();
    static char **to_c_array(std::vector<std::string> const &vec);
    static void free_c_array(char **array);
};

#endif
