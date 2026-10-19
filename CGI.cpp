#include "CGI.hpp"

CGI::CGI(HTTP &other) : env(other.get_env()), method(other.get_method()),\
file(other.get_pwd()), protocol(other.get_protocol()), host(other.get_value_host()), \
fd(other.get_fd())
{
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
    basic_envp();
    script_name();
    extract_host_port();
}

void    CGI::fork_program()
{

}

void    CGI::basic_envp()
{
    envp.push_back("GATEWAY_INTERFACE=CGI/1.1");
    envp.push_back("REQUEST_METHOD=" + method);
    envp.push_back("PROTOCOL_VERSION=" + protocol);
    envp.push_back("SERVER_SOFTWARE=webserv/1.0");
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
    envp.push_back("SCRIPT_NAME=" + filename);
    envp.push_back("QUERY_STRING=" + vars);
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
    envp.push_back("SERVER_NAME=" + hostname);
    envp.push_back("SERVER_PORT=" + port);
}