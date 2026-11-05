#include "ExecuteRequest.hpp"
#include "ErrorCodes.hpp"
#include "FindServer.hpp"

std::string const ExecuteRequest::get_content(t_server const &serv, t_location const *loc, t_header * head)
{
    if (loc->cgi_pass == false)
        complete_file_path(serv, loc, head);
    
    exec_info.output = get_output(serv, loc, head);
    //complete file path with autoindex index and default path (n'a de sens que si pas un cgi)

    //find autor
}

void    ExecuteRequest::complete_file_path(t_server const &serv, t_location const *loc, t_header * head)
{
    //dans un premier temps completer l'URI en mettant la partie avant
    if (loc->file_directory != "")
        exec_info.filename = loc->file_directory + head->URI;
    else if (serv.server_repo != "")
        exec_info.filename = serv.server_repo + head->URI;

    if (head->URI.empty())
        throw(ErrorCodes(403));
    if (head->URI[head->URI.size() - 1] == '/')
        complete_repo(serv, loc, head);
    else 
        complete_file(serv, loc, head);

    //dans un second temps il faut append les index de fichier potentiel si la requete ne contient pas un 
    //fichier de base 
    
}

void    ExecuteRequest::complete_repo(t_server const &serv, t_location const *loc, t_header * head)
{
    struct stat arg2;

    if (stat(exec_info.filename.c_str(), &arg2) == -1 || !S_ISDIR(arg2.st_mode))
        throw(ErrorCodes(404));
    if (loc->default_file != "")
    {
        exec_info.filename += loc->default_file;
        return; 
    }
    if (loc->autoindex)
    {
        exec_info.filename += AUTOINDEX;
        return ;
    }
}

void    ExecuteRequest::complete_file(t_server const &serv, t_location const *loc, t_header * head)
{
    struct stat arg2;

    if (stat(exec_info.filename.c_str(), &arg2) == -1)
        throw(ErrorCodes(404));
    if (S_ISREG(arg2.st_mode))
        return ;
    if (S_ISDIR(arg2.st_mode))
        throw(ErrorCodes(301, head->URI + '/'));
    throw(ErrorCodes(500));
}

std::string ExecuteRequest::get_output(t_server const &serv, t_location const *loc, t_header *head)
{
    
    //Ici il faut gerer les Get Post Delete et CGI
}