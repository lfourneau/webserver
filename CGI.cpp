#include "CGI.hpp"
#include "HelperFunctions.hpp"
#include <wait.h>

CGI::CGI()
{
}

CGI::CGI(CGI const &other) : cgi(other.cgi)
{
    cgi.argv = NULL;
    cgi.envp = NULL;
    transform_to_c_str(this->cgi.argv_str, this->cgi.argv);
    transform_to_c_str(this->cgi.content, this->cgi.envp);
}

CGI &CGI::operator=(CGI const &other)
{
    if (this != &other)
    {
        free_char_lists(cgi.argv);
        free_char_lists(cgi.envp);
        this->cgi = other.cgi;
        cgi.argv = NULL;
        cgi.envp = NULL;
        transform_to_c_str(this->cgi.argv_str, this->cgi.argv);
        transform_to_c_str(this->cgi.content, this->cgi.envp);
    }
    return (*this);
}

CGI::~CGI()
{
    free_char_lists(cgi.argv);
    free_char_lists(cgi.envp);
}

void CGI::run(t_execute *exec, t_header const *head, t_location const *loc)
{
    CGI cgi_exec;

    cgi_exec.extract_output(exec, head, loc);
}

void CGI::extract_output(t_execute *exec, t_header const *head,\
     t_location const *loc)
{
    if (loc->cgi_pass != true)
        throw(ErrorCodes(500));
    make_headers(exec, head, loc);
    make_char_stars(head, loc);
    fork_program(exec, head, loc);
}

void CGI::make_headers(t_execute *exec, t_header const *head,\
t_location const *loc)
{
    transform_client_headers(head);
    add_additionnal_headers(exec, head, loc);
}

void CGI::transform_client_headers(t_header const *head)
{
    std::map<std::string, std::string>::const_iterator it = head->headers.begin();

    for (; it != head->headers.end(); ++it)
    {
        std::string copy_of_header_name;

        copy_of_header_name = it->first;
        std::replace(copy_of_header_name.begin(), copy_of_header_name.end(), '-', '_');
        HelperFunctions::maxi(copy_of_header_name);
        copy_of_header_name.append("=");
        copy_of_header_name.append(it->second);
        copy_of_header_name.append("\r\n");
        if (it->first != "content-type" || it->second != "content-length")
            copy_of_header_name.insert(0, "HTTP_");
        cgi.content.push_back(copy_of_header_name); 
    }
}

void CGI::add_additionnal_headers(t_execute *exec, t_header const *head, \
t_location const *loc)
{
    cgi.content.push_back("REQUEST_METHOD=" + head->method + "\r\n");
    cgi.content.push_back("SCRIPT_NAME=" + loc->program_exec_cgi + "\r\n");
    cgi.content.push_back("SCRIPT_FILENAME=" + exec->filename + "\r\n");
    //will have to check content += "PATH_INFO=" + "\r\n"; //not sure what this is
    cgi.content.push_back("QUERY_STRING=" + head->cgi_extension + "\r\n");
    cgi.content.push_back("SERVER_PROTOCOL=" + head->protocol + "\r\n");
    cgi.content.push_back("SERVER_NAME=" + head->host_name + "\r\n");
    cgi.content.push_back("SERVER_PORT=" + HelperFunctions::ft_itoa(head->host_port) + "\r\n");
    cgi.content.push_back("GATEWAY_INTERFACE=CGI/1.1\r\n");
    cgi.content.push_back("REMOTE_ADDR=127.0.0.1\r\n");
    cgi.content.push_back("REDIRECT_STATUS=200\r\n"); 
}

void CGI::make_char_stars(t_header const *head, t_location const *loc)
{
    create_argv(head, loc);
    transform_to_c_str(cgi.argv_str, cgi.argv);
    transform_to_c_str(cgi.content, cgi.envp);
}


void    CGI::transform_to_c_str(std::vector<std::string> const &vec, char **&dest)
{
    free_char_lists(dest);

    if (vec.size() == 0)
        return;
    
    dest = new char *[vec.size() + 1];

    std::vector<std::string>::const_iterator it;
    unsigned int i = 0;

    for (it = vec.begin(); it != vec.end(); it++)
    {
        char *copy = new char[it->size() + 1];

        std::strcpy(copy, it->c_str());
        dest[i] = copy;
        i++;
    }
    dest[i] = NULL;
}

void    CGI::create_argv(t_header const *head, t_location const *loc)
{
    cgi.argv_str.push_back(loc->program_exec_cgi);
    cgi.argv_str.push_back(head->file_name);
}

void    CGI::free_char_lists(char **&cont)
{
    if (cont == NULL)
        return ;
    
    unsigned int i = 0;
    while (cont[i] != NULL)
    {
        delete[] cont[i];
        cont[i] = NULL;
        i++;
    }
    delete[] cont;
    cont = NULL;
}
void    CGI::fork_program(t_execute *exec, t_header const *head, t_location const *loc)
{
    int to_file[2];
    int to_server[2];
    pid_t child;
    char buffer[4096];

    pipe(to_file);
    pipe(to_server);
    child  = fork();
    if (child == 0) 
    {
        dup2(to_file[0], STDIN_FILENO);
        dup2(to_server[1], STDOUT_FILENO);
        close(to_file[0]);
        close(to_file[1]);
        close(to_server[0]);
        close(to_server[1]);
        execve(loc->program_exec_cgi.c_str(), &cgi.argv[0], &cgi.envp[0]);
    }
    close(to_file[0]);
    write(to_file[1], head->body_request.c_str(), head->body_request.size());
    close(to_file[1]);

    close(to_server[1]);
    size_t total = 0;
    while (true)
    {
        bzero(buffer, 4096);
        total = read(to_server[0], buffer, 4096);
        if (total == 0)
            break;
        exec->output.append(buffer, total);
    }
    close(to_server[0]);
    int status;
    waitpid(child, &status, 0);
}

/*
void    CGI::basic_envp(std::vector<std::string> &vec)
{
    vec.push_back(TransformToString() << "SERVER_NAME=" << info_request["Host_name"]);
    vec.push_back(TransformToString() << "SERVER_PORT=" << info_request["Host_port"]);
    vec.push_back("HTTP_GATEWAY_INTERFACE=CGI/1.1");
    vec.push_back(TransformToString() << "HTTP_REQUEST_METHOD=" << info_request["Method"]);
    vec.push_back(TransformToString() << "HTTP_PROTOCOL_VERSION=" << info_request["Protocol"]);
    vec.push_back("HTTP_SERVER_SOFTWARE=webserv/1.0");
    vec.push_back(TransformToString() << "HTTP_REMOTE_ADDR=" << associated_fd);
}
*/

/*
CGI::CGI(HTTP const &other) : HTTP(other) 
{
    exec_program = "/usr/bin/python3";
}

CGI::CGI(CGI const &other) : HTTP(other)
{
    (void)other;
}

CGI &CGI::operator=(CGI const &other)
{
    (void)other;
    return (*this);
}

CGI::~CGI() throw()
{
    free_char_lists();
}

void    CGI::parse_input()
{ 
    std::vector<std::string> string_vector;
    script_name(string_vector);
    sup_post(string_vector);
    basic_envp(string_vector);
    transform_to_c_str(string_vector);
    create_argv();
}

void    CGI::fork_program()
{
    int to_file[2];
    int to_server[2];
    pid_t child;
    char buffer[4096];

    pipe(to_file);
    pipe(to_server);
    child  = fork();
    if (child == 0) 
    {
        dup2(to_file[0], STDIN_FILENO);
        dup2(to_server[1], STDOUT_FILENO);
        close(to_file[0]);
        close(to_file[1]);
        close(to_server[0]);
        close(to_server[1]);
        execve(exec_program.c_str(), &argv[0], &envp[0]);
    }
    close(to_file[0]);
    write(to_file[1], body.c_str(), body.size());
    close(to_file[1]);

    close(to_server[1]);
    size_t total = 0;
    while (true)
    {
        bzero(buffer, 4096);
        total = read(to_server[0], buffer, 4096);
        if (total == 0)
            break;
        output_cgi.append(buffer, total);
    }
    close(to_server[0]);
    int status;
    waitpid(child, &status, 0);
}

void    CGI::basic_envp(std::vector<std::string> &vec)
{
    vec.push_back(TransformToString() << "SERVER_NAME=" << info_request["Host_name"]);
    vec.push_back(TransformToString() << "SERVER_PORT=" << info_request["Host_port"]);
    vec.push_back("HTTP_GATEWAY_INTERFACE=CGI/1.1");
    vec.push_back(TransformToString() << "HTTP_REQUEST_METHOD=" << info_request["Method"]);
    vec.push_back(TransformToString() << "HTTP_PROTOCOL_VERSION=" << info_request["Protocol"]);
    vec.push_back("HTTP_SERVER_SOFTWARE=webserv/1.0");
    vec.push_back(TransformToString() << "HTTP_REMOTE_ADDR=" << associated_fd);
}

void    CGI::script_name(std::vector<std::string> &vec)
{
    info_request["File"] = HelperFunctions::trim(info_request["File"], " \r\n\t");
    info_request["CGI params"] = HelperFunctions::trim(info_request["CGI params"], " \r\n\t");
    vec.push_back(TransformToString() << "HTTP_SCRIPT_NAME=" << info_request["File"]);
    vec.push_back(TransformToString() << "HTTP_QUERY_STRING=" << info_request["CGI params"]);
}

void    CGI::sup_post(std::vector<std::string> &vec)
{
    if (info_request["Method"] == "POST")
    {
        vec.push_back(TransformToString() << "CONTENT_LENGTH=" << content.size());
        vec.push_back(TransformToString() << "CONTENT_TYPE=" << find_type());
    
    }
}

std::string const CGI::find_type() const
{
    //le serveur n'accepte que du python pour l'instant donc on se contente de ca

    return  ("text/x-python");
}

std::string CGI::get_output()
{
    parse_input();
    fork_program();
    return (output_cgi);
}

void    CGI::transform_to_c_str(std::vector<std::string> const &vec)
{
    free_char_lists();

    std::vector<std::string>::const_iterator it;

    for (it = vec.begin(); it != vec.end(); it++)
    {
        char *copy = new char[it->size() + 1];

        std::strcpy(copy, it->c_str());
        envp.push_back(copy);
    }
    envp.push_back(NULL);
}

void    CGI::create_argv()
{
    argv.push_back(const_cast<char *>(exec_program.c_str()));
    argv.push_back(const_cast<char *>(info_request["File"].c_str()));
    argv.push_back(NULL);
}

void    CGI::free_char_lists()
{
    for (long unsigned int i = 0; i < envp.size() ; i++)
    {
        delete envp[i];
        envp[i] = NULL;
    }
}*/