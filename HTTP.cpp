#include "HTTP.hpp"
#include "CGI.hpp"
#include "Post.hpp"
#include "ErrorCodes.hpp"
#include "Delete.hpp"

//Les constructeurs

HTTP::HTTP(int fd, t_serverconfig const &serv) : associated_fd(fd), pwd(PATH_FILES_DIRECTORY), \
len_body(-1), cgi_request(false), code(200), error_message("OK"),\
headers_parsed(false), server_rules(serv), bloc_found(false)
{
}

HTTP::HTTP(const HTTP &other) : server_rules(other.server_rules)
{
    this->associated_fd = other.associated_fd;
    this->request_content = other.request_content;
    this->request_method = other.request_method;
    this->URI = other.URI;
    this->protocol_version = other.protocol_version;
    this->pwd = other.pwd;
    this->content = other.content;
    this->host = other.host;
    this->host_port = other.host_port;
    this->file_type = other.file_type;
    this->query_string = other.query_string;
    this->content_type = other.content_type;
    this->body = other.body;
    this->len_body = other.len_body;
    this->cgi_request = other.cgi_request;
    this->code = other.code;
    this->header_lines = other.header_lines;
    this->error_message = other.error_message;
    this->bloc_found = other.bloc_found;
    this->loc = other.loc;
    this->headers_parsed = other.headers_parsed;
}

HTTP    &HTTP::operator=(const HTTP &other)
{
    if (this != &other)
    {
        this->associated_fd = other.associated_fd;
        this->request_content = other.request_content;
        this->request_method = other.request_method;
        this->URI = other.URI;
        this->protocol_version = other.protocol_version;
        this->pwd = other.pwd;
        this->content = other.content;
        this->host = other.host;
        this->host_port = other.host_port;
        this->file_type = other.file_type;
        this->query_string = other.query_string;
        this->content_type = other.content_type;
        this->body = other.body;
        this->len_body = other.len_body;
        this->cgi_request = other.cgi_request;
        this->code = other.code;
        this->header_lines = other.header_lines;
        this->error_message = other.error_message;
        this->bloc_found = other.bloc_found;
        this->loc = other.loc;
        this->headers_parsed = other.headers_parsed;
    }
    return (*this);
}

HTTP::~HTTP()
{

}

//HelperFunctions not usefull here
int  HTTP::get_client_fd()
{
    return (this->associated_fd);
}

void HTTP::print_content()
{
    std::cout << request_content;
}

void    HTTP::get_value_host()
{
    std::string host_val;
   
    try
    {
        host_val = header_lines.at("Host");
    }
    catch(const std::exception& e)
    {
        (void)(e);
        throw(ErrorCodes(400));
    }
    std::stringstream info;

    info << host_val;
    getline(info, host, ':');
    getline(info, host_port);
}

//
//Fonctionnement de la classe
//recuperer le contenu envoyer par chunk par le serveur
//parser premiere linge
//sauvegarder sous forme de paire le registe
//ouvrir le fichier associer
//l'envoyer

int HTTP::append_content(const char *append_info, unsigned int total_read)
{
    request_content.append(append_info, total_read);
    if (this->request_content.find("\r\n\r\n") != std::string::npos)
    {
        try
        {
            if (headers_parsed == false)
            {
                analyse_buffer();
                headers_parsed = true;
            }
            if (all_body_received())
                perform_request();
            else 
                return (0);
        }
        catch(const ErrorCodes &e)
        {
            std::string err = e.what();
            int val = e.getcode();
            std::cerr << "Error: " << err << '\n';
            build_error(val, err);
            cgi_content();
        }
        encapsulate_response();
        close(associated_fd);
        return (1);
    }
    return (0);
}

bool    HTTP::all_body_received()
{
    if (request_method == "GET" || (request_method == "POST" && \
        cgi_request == true) || request_method == "DELETE")
        return (true);

    if (request_method == "POST")
    {
        if (len_body == -1)
        {
            std::map<std::string, std::string>::iterator it;

            it = header_lines.find("Content-Length");
            if (it == header_lines.end())
                throw (ErrorCodes(400));
            transform_to_num(it->second, len_body);
        }
        if (static_cast<long unsigned int>(len_body) == body.size())
            return (true);
        return (false);
    }
    return (true);
}

void HTTP::analyse_buffer()
{
    //dans cette commande il faudrait peut etre 
    //recuperer le contenu de ma requete puis ensuite proceder a une
    //encapsulation
    std::stringstream flux;
    std::string server_response;

    flux << this->request_content;
    parse_initial_response_line(flux);
    parse_header_lines(flux);
    http_body(flux);
    get_value_host();
    find_bloc_request();
}

void    HTTP::build_error(int error_code, std::string meaning)
{
    //on doit modifier le contenu de la classe HTTP pcq elle peut crash a n'importe quel moment
    this->request_method = "POST";
    this->URI = "/cgi-bin/error_page.py";
    this->protocol_version = "HTTP/1.0";
    this->pwd = "/home/lfournea/Documents/webserver/cgi-bin/error_page.py";
    this->content = TransformToString() << "code=" << error_code;
    this->host = "localhost";
    this->host_port = "5600";
    this->file_type = "text/html";
    this->query_string = "";
    this->content_type = "text/html";
    this->body = TransformToString() << "code=" << error_code;
    this->cgi_request = true;
    this->code = error_code;
    this->error_message = meaning;
}

void    HTTP::perform_request()
{
    //il faudra creer des strucutes qui autorisent qqn d'externe a lire ecrire ou executer 
    //un document
    if (request_method == "GET" || (request_method == "POST" && cgi_request == true))
    {
        if (!check_authorized_method(request_method))
            throw(ErrorCodes(403));
        if (cgi_request == false)
            read_file();
        else
        {
            pwd += URI;
            cgi_content();
        }
    }
    else if (request_method == "POST")
    {
        Post post_file(*this);

        post_file.request_post();
    }
    else if (request_method == "DELETE")
    {
        Delete delete_file(*this);

        delete_file.request_delete();
    }
    else 
        throw(ErrorCodes(405));
}

bool HTTP::check_authorized_method(std::string const &val)
{
    if (!find_location_block(loc))
        return (false);
    if (val == "GET" && loc.get == true)
        return (true);
    else if (val == "POST" && loc.post == true)
        return (true);
    else if (val == "DELETE" && loc.del == true)
        return (true);
    return (false);
}

bool HTTP::find_location_block(t_location &find)
{
    int longest_match = 0;
    size_t pos;

    for (std::vector<s_location>::iterator it = this->bloc.location_blocks.begin(); \
    it != this->bloc.location_blocks.end(); \
    ++it)
    {
        pos = URI.compare(0, it->prefix.size(), it->prefix);
        if (pos == 0)
        {
            if (static_cast<int>(it->prefix.size()) > longest_match)
            {
                longest_match = static_cast<int>(it->prefix.size());
                find = *it;
            }
        }
    }
    if (longest_match != 0)
        return (true);
    throw(ErrorCodes(404));
    return (false);
}

void HTTP::parse_initial_response_line(std::stringstream &flux)
{
    std::string first_line;

    if (!getline(flux, first_line))
    {
            throw(ErrorCodes(400));
    }
    std::istringstream input(first_line);
    std::string extra_values;

    // la synthaxe renvoie vrai si on a pu lire et faux dans le cas contraire
    if (!(input >> request_method >> URI >> protocol_version) || (input >> extra_values))
    {
        throw(ErrorCodes(400));
    }
    check_cgi_request();
}

void HTTP::parse_header_lines(std::stringstream &flux)
{
    while (true)
    {
        std::string current_line;
        std::getline(flux, current_line);

        if (current_line.empty() == true || HelperFunctions::isblank(current_line))
            break;
        if (current_line.empty() == false && (current_line[0] == ' ' || current_line[0] == '\t'))
        {
            add_to_previous_pair(current_line);
            continue;
        }

        std::istringstream line_cut(current_line);
        std::string key;
        std::string value;

        //attention cela ne marche que dans le cadre de paires
        //si c'est une liste il faut separer avec le ":"
        //ici il faudrait peut etre verifier la presence de ce charactere :
        getline(line_cut, key, ':');
        if (line_cut.eof())
        {
            std::cout << "Pas de colon identifie" << std::endl;
            return;
        }
        getline(line_cut, value);
        value = HelperFunctions::trim(value, "\r ");
        if (key.empty() == true || value.empty() == true)
            throw(ErrorCodes(400));
        this->header_lines[key] =  value;
    }
}

void    HTTP::add_to_previous_pair(std::string &content)
{

    if (this->header_lines.empty() == true)
    {
        std::cout << "there is an error with the request";
        return;
    }

    std::map<std::string, std::string>::reverse_iterator it = header_lines.rbegin();
    it->second.append(content);
}

void    HTTP::http_body(std::stringstream &flux)
{
    std::ostringstream tmp;
    tmp << flux.rdbuf();
    body = tmp.str();
}

// ici on lit et on stocke le contenu du fichier dans une string
void    HTTP::read_file()
{
    int file_desc;

    modify_route(pwd);
    if (!contains_a_file() && this->pwd.back() == '/')
    {
        pwd += '/';
        throw(ErrorCodes(301));
    }
    if (!contains_a_file() && this->loc.default_file != "")
    {
        pwd += this->loc.default_file;
        determine_extension();
    }
    else if (!contains_a_file() && this->loc.autoindex == true)
    {
        //ici il faut envoyer un fichier qui montre tous les
        //de l'index
    }
    else if (!contains_a_file() && this->loc.autoindex == false)
    {
        throw (ErrorCodes(403));
    }
    file_desc = open(pwd.c_str(), O_RDONLY);
    if (file_desc == -1)
        throw(ErrorCodes(404));
    read_and_store_file(file_desc);
}

bool HTTP::contains_a_file()
{
    size_t rightmost_dash;
    size_t rightmost_dot;

    if (this->cgi_request == true)
        return (true);
    rightmost_dash = pwd.rfind("/");
    rightmost_dot = pwd.rfind(".");

    if (rightmost_dot != std::string::npos && rightmost_dot > rightmost_dash)
        return (true);
    return (false);
}

void HTTP::modify_route(std::string &ref)
{
    ref = TransformToString() << this->loc.file_directory << URI;
    return;
}

bool    HTTP::check_cgi_request()
{
    if (URI.find("?") == std::string::npos)
        return(false);

    std::stringstream line;
    line << URI;
    URI = "";
    URI.clear();
    getline(line, URI, '?');
    getline(line, query_string);
    return (true);
}

void    HTTP::determine_extension()
{
    size_t last_slash;
    size_t exstension;
    std::string file_exstensions[] = {"html", "htm", "css", "js", "txt", "json", "png", "jpg", \
        "jpeg", "gif", "svg", "ico", "pdf", "py"};
    std::string file_type[] = {"text/html", "text/html", "text/css", "text/javascript", "text/plain", \
        "application/json", "image/png", "image/jpeg", "image/jpeg", "image/gif", "image/svg+xml", \
        "image/x-ico", "application/pdf", "text/html" };

    last_slash = pwd.rfind('/');
    exstension = pwd.rfind('.');
    if (exstension == std::string::npos && last_slash != std::string::npos)
    {
        content_type = std::string("application/octet-stream");
        return ;
    }
    for (int i = 0; static_cast<unsigned long>(i) < sizeof(file_exstensions) / sizeof(file_exstensions[0]); i++)
    {
        if (URI.find(file_exstensions[i]) != std::string::npos)
        {
            content_type = file_type[i];
            if (i > 12)
                cgi_request = true;
            return ;
        }
    }
    //attention ne fonctionne pas si c'est un fichier .php
    content_type = std::string("text/html");
    
}

void    HTTP::read_and_store_file(int fd)
{
    char    buffer[4096];
    int  current_buffer;

    bzero(buffer, 4096);
    while (true)
    {
        current_buffer = read(fd, buffer, 4096);
        if (current_buffer == 0)
        {
            close (fd);
            break;
        }
        content.append(buffer, current_buffer);
        bzero(buffer, 4096);
    }
}

//on utilise le contenu de cette string pour constuire une reponse 

void    HTTP::cgi_content(void)
{
    std::ostringstream output_message;
    std::string final_content;

    if (cgi_request)
    {
        CGI     send_to_cgi(*this);

        content = send_to_cgi.get_output();
    }
    return;
}

void    HTTP::encapsulate_response()
{
    std::ostringstream output_message;
    std::string final_message;

    output_message << "HTTP/1.0 " << code << " " << this->error_message << "\r\n" \
    << "Content-Type: " << content_type << "\r\n" << "Content-Length: " << content.size() << "\r\n\r\n" \
    << content;
    final_message = output_message.str();
    send(this->associated_fd, final_message.c_str(), final_message.size(), 0);
    return ;
}

void    HTTP::find_bloc_request()
{
    std::vector<t_server>::const_iterator it = this->server_rules.servers.begin();

    for (; it != this->server_rules.servers.end(); ++it)
    {
        int port_to_compare = -1;

        for (std::set<int>::iterator val = it->open_ports.begin(); val != it->open_ports.end(); ++val )
        {
            int port_num;
            transform_to_num(host_port, port_num);
            if (*val == port_num)
            {
                port_to_compare = port_num;
                break;
            }
        }
        if (port_to_compare == -1)
            continue;
        if (bloc_found == false)
        {
            bloc = *it;
            bloc_found = true;
            continue;
        }
        for (std::vector<std::string>::const_iterator server_names = it->server_names.begin(); \
            server_names != it->server_names.end(); ++server_names)
        {
            if (*server_names == host)
            {
                bloc = *it;
                bloc_found = true;
                return;
            }
        }
    }
    if (bloc_found == false)
        throw (ErrorCodes(500));
}