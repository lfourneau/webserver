#include "ExecuteRequest.hpp"
#include "ErrorCodes.hpp"
#include "Get.hpp"
#include "Post.hpp"
#include "Delete.hpp"
#include "CGI.hpp"

ExecuteRequest::ExecuteRequest(t_server const &serv, t_location const &loc, t_header const &head) : \
serv(&serv), loc(&loc), head(&head), exec_info()
{
}

ExecuteRequest::ExecuteRequest(ExecuteRequest const &other) : serv(other.serv), \
loc(other.loc), head(other.head), exec_info(other.exec_info)
{
}

ExecuteRequest &ExecuteRequest::operator=(ExecuteRequest const &other)
{
    if (this != &other)
    {
        serv = other.serv;
        loc = other.loc;
        head = other.head;
        exec_info = other.exec_info;
    }
    return (*this);
}

ExecuteRequest::~ExecuteRequest()
{
}

void    ExecuteRequest::execute()
{
    typedef void (*t_handler)(t_execute *, t_header const *, t_location const *);
    t_handler const handlers[] = {&Get::extract_output, &Post::post_file, &Delete::delete_file};
    int method;

    if (loc->redirect_code != 0)
        throw ErrorCodes(loc->redirect_code, loc->redirect_url);
    method = find_method();
    build_path();
    if (loc->cgi_pass)
    {
        struct stat info;

        if (stat(exec_info.filename.c_str(), &info) == -1 || !S_ISREG(info.st_mode))
            throw ErrorCodes(404);
        exec_info.cgi = true;
        exec_info.cgi_prog = loc->program_exec_cgi;
        exec_info.cgi_script = exec_info.filename;
        exec_info.cgi_env = CGI::request_env(&exec_info, head);
    }
    else
    {
        if (method == 0)
        {
            if (head->URI[head->URI.size() - 1] == '/')
                complete_repo();
            else
                complete_file();
        }
        if (exec_info.autoindex)
            prepare_autoindex();
        else
            handlers[method](&exec_info, head, loc);
    }
}

int ExecuteRequest::get_code() const
{
    return (exec_info.code);
}

std::string const &ExecuteRequest::get_output() const
{
    return (exec_info.output);
}

std::string const &ExecuteRequest::get_filename() const
{
    return (exec_info.filename);
}

bool ExecuteRequest::is_autoindex() const
{
    return (exec_info.autoindex);
}

t_execute const &ExecuteRequest::get_exec() const
{
    return (exec_info);
}

int ExecuteRequest::find_method() const
{
    std::string const methods[] = {"GET", "POST", "DELETE"};
    bool const allowed[] = {loc->get, loc->post, loc->del};

    for (int i = 0; i < 3; i++)
    {
        if (head->method == methods[i])
        {
            if (!allowed[i])
                throw ErrorCodes(405);
            return (i);
        }
    }
    throw ErrorCodes(501);
}

void    ExecuteRequest::build_path()
{
    if (head->URI.empty())
        throw ErrorCodes(403);
    if (loc->file_directory != "" && !loc->cgi_pass)
    {
        std::string rest = head->URI.substr(loc->prefix.size());

        if (rest.empty() || rest[0] != '/')
            rest.insert(0, "/");
        exec_info.filename = loc->file_directory + rest;
    }
    else if (loc->file_directory != "")
        exec_info.filename = loc->file_directory + head->URI;
    else if (serv->server_repo != "")
        exec_info.filename = serv->server_repo + head->URI;
    else
        throw ErrorCodes(500);
}

void    ExecuteRequest::complete_repo()
{
    struct stat info;

    if (stat(exec_info.filename.c_str(), &info) == -1 || !S_ISDIR(info.st_mode))
        throw ErrorCodes(404);
    if (this->head->method == "POST")
        return;
    if (loc->default_file != ""
        && stat((exec_info.filename + loc->default_file).c_str(), &info) == 0
        && S_ISREG(info.st_mode))
        exec_info.filename += loc->default_file;
    else if (loc->autoindex)
        exec_info.autoindex = true;
    else
        throw ErrorCodes(403);
}

void    ExecuteRequest::prepare_autoindex()
{
    exec_info.cgi = true;
    exec_info.cgi_prog = PYTHON_INTERPRETER;
    exec_info.cgi_script = serv->server_repo + "/" + AUTOINDEX_SCRIPT;
    exec_info.cgi_env.push_back("DIRECTORY=" + exec_info.filename);
    exec_info.cgi_env.push_back("REQUEST_URI=" + head->URI);
    exec_info.cgi_env.push_back("REQUEST_METHOD=GET");
    exec_info.cgi_env.push_back("GATEWAY_INTERFACE=CGI/1.1");
    exec_info.cgi_env.push_back("SCRIPT_FILENAME=" + exec_info.cgi_script);
}

void    ExecuteRequest::complete_file()
{
    struct stat info;

    if (stat(exec_info.filename.c_str(), &info) == -1)
        throw ErrorCodes(404);
    if (S_ISREG(info.st_mode))
        return ;
    if (S_ISDIR(info.st_mode))
        throw ErrorCodes(head->method == "GET" ? 301 : 308, head->URI + '/');
    throw ErrorCodes(500);
}
