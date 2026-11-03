#include "CGI.hpp"

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
}