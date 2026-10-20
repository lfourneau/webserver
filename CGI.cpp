#include "CGI.hpp"

CGI::CGI(HTTP const &other) : HTTP(other) 
{
    exec_program = "/usr/bin/python3";
}

CGI::CGI(CGI const &other) : HTTP(0)
{
    (void)other;
}

CGI &CGI::operator=(CGI const &other)
{
    (void)other;
    return (*this);
}

CGI::~CGI()
{

}

void    CGI::parse_input()
{ 
    std::vector<std::string> string_vector;
    extract_host_port(string_vector);
    script_name(string_vector);
    basic_envp(string_vector);
    transform_to_c_str(string_vector);
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
        char **vide = NULL;
       dup2(to_file[0], STDIN_FILENO);
       dup2(to_server[1], STDOUT_FILENO);
       close(to_file[0]);
       close(to_file[1]);
       close(to_server[0]);
       close(to_server[1]);
       execve(exec_program.c_str(), vide, &envp[0]);
    }
    close(to_file[0]);
    write(to_file[1], content.c_str(), content.size());
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
    vec.push_back("HTTP_GATEWAY_INTERFACE=CGI/1.1");
    vec.push_back(TransformToString() << "HTTP_REQUEST_METHOD=" << request_method);
    vec.push_back(TransformToString() << "HTTP_PROTOCOL_VERSION=" << protocol_version);
    vec.push_back("HTTP_SERVER_SOFTWARE=webserv/1.0");
    vec.push_back(TransformToString() << "HTTP_REMOTE_ADDR=" << associated_fd);
    vec.push_back(NULL);
}

void    CGI::script_name(std::vector<std::string> &vec)
{
    std::stringstream ss_pwd;
    std::string filename;
    std::string vars;

    ss_pwd << pwd;

    getline(ss_pwd, filename, '?');
    getline(ss_pwd, vars);
    filename = HelperFunctions::trim(filename, " \r\n\t");
    vars = HelperFunctions::trim(vars, " \r\n\t");
    vec.push_back(TransformToString() << "HTTP_SCRIPT_NAME=" << filename);
    vec.push_back(TransformToString() << "HTTP_QUERY_STRING=" << vars);
}

void    CGI::extract_host_port(std::vector<std::string> &vec)
{
    std::stringstream content;
    std::string hostname;

    content << host;
    getline(content, hostname, ':');
    hostname = HelperFunctions::trim(hostname, " \r\n");
    vec.push_back(TransformToString() << "SERVER_NAME=" << hostname);
    vec.push_back(TransformToString() << "SERVER_PORT=" << host_port);
}

void    CGI::sup_post(std::vector<std::string> &vec)
{
    if (request_method == "POST")
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
    std::vector<std::string>::const_iterator it;

    for (it = vec.begin(); it != vec.end(); it++)
    {
        char *copy;

        copy = const_cast<char *>((*it).c_str());
        envp.push_back(copy);
    }
}