#include "CGI.hpp"

CGI::CGI(HTTP &other) : env(other.get_env()), method(other.get_method()),\
file(other.get_pwd()), protocol(other.get_protocol()), host(other.get_value_host()), \
fd(other.get_fd()), content(other.get_content())
{
    exec_program = "/usr/bin/python3";
}

CGI::CGI(CGI const &other)
{

}

CGI &CGI::operator=(CGI const &other)
{
    return (*this);
}

CGI::~CGI()
{

}

void    CGI::parse_input()
{ 
    extract_host_port();
    script_name();
    basic_envp();
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
       execve(exec_program.c_str(), NULL, &envp[0]);
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
        if (total = 0)
            break;
        output_cgi.append(buffer, total);
    }
    close(to_server[0]);
    int status;
    waitpid(child, &status, 0);
}

void    CGI::basic_envp()
{
    envp.push_back("HTTP_GATEWAY_INTERFACE=CGI/1.1");
    envp.push_back(to_c_str("HTTP_REQUEST_METHOD=", method));
    envp.push_back(to_c_str("HTTP_PROTOCOL_VERSION=", protocol));
    envp.push_back("HTTP_SERVER_SOFTWARE=webserv/1.0");
    envp.push_back(to_c_str("HTTP_REMOTE_ADDR=" , fd));
    envp.push_back(NULL);
}

void    CGI::script_name()
{
    std::stringstream pwd;
    std::string filename;
    std::string vars;

    pwd << file;

    getline(pwd, filename, '?');
    getline(pwd, vars);
    filename = HelperFunctions::trim(filename, " \r\n\t");
    vars = HelperFunctions::trim(vars, " \r\n\t");
    envp.push_back(to_c_str("HTTP_SCRIPT_NAME=" , filename));
    envp.push_back(to_c_str("HTTP_QUERY_STRING=" , vars));
}

void    CGI::extract_host_port()
{
    std::stringstream content;

    std::string port;
    std::string hostname;

    content << host;
    getline(content, hostname, ':');
    getline(content, port);
    port = HelperFunctions::trim(hostname, " \r\n");
    hostname = HelperFunctions::trim(port, " \r\n");
    envp.push_back(to_c_str("SERVER_NAME=" , hostname));
    envp.push_back(to_c_str("SERVER_PORT=" , port));
}

void    CGI::sup_post()
{
    if (method == "POST")
    {
        envp.push_back(to_c_str("CONTENT_LENGTH=" , content.size()));
        envp.push_back(to_c_str("CONTENT_TYPE=" , find_type()));
    }
}

std::string const CGI::find_type() const
{
    //le serveur n'accepte que du python pour l'instant donc on se contente de ca

    return  ("text/x-python");
}

std::string CGI::get_output()
{
    return (this->output_cgi);
}