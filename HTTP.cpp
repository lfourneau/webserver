#include "HTTP.hpp"
#include "CGI.hpp"
#include "Post.hpp"
#include "ErrorCodes.hpp"

//Les constructeurs

HTTP::HTTP(int fd) : associated_fd(fd), pwd(PATH_FILES_DIRECTORY), cgi_request(false), code(200), error_message("OK")
{
}

HTTP::HTTP(const HTTP &other)
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
    this->cgi_request = other.cgi_request;
    this->code = other.code;
    this->header_lines = other.header_lines;
    this->error_message = other.error_message;
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
        this->cgi_request = other.cgi_request;
        this->code = other.code;
        this->header_lines = other.header_lines;
        this->error_message = other.error_message;
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
    std::map<std::string, std::string>::iterator host_val = header_lines.find("Host");
    std::stringstream info;

    if (host_val == header_lines.end())
    {
        std::cout << "Error with the host header line" << std::endl;
        return ;
    }
    info << host_val->second;
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
        analyse_buffer();
        return (1);
    }
    return (0);
}

void HTTP::analyse_buffer()
{
    //dans cette commande il faudrait peut etre 
    //recuperer le contenu de ma requete puis ensuite proceder a une
    //encapsulation
    std::stringstream flux;
    std::string server_response;

    flux << this->request_content;
    try
    {
        parse_initial_response_line(flux);
        parse_header_lines(flux);
        http_body(flux);
        get_value_host();
        determine_extension();
        perform_request();
    }
    catch(const ErrorCodes& e)
    {
        build_error(e.getcode(), e.getmessage());
        cgi_content();
    }
    encapsulate_response();
    close(associated_fd);
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
    this->host_port = "5400";
    this->file_type = "text/html";
    this->query_string = "";
    this->content_type = "text/html";
    this->body = "";
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
        if (cgi_request == false)
            read_file();
        else
            cgi_content();
    }
    else if (request_method == "POST")
    {
        Post post_file(*this);

        post_file.request_post();
    }
    else if (request_method == "DELETE")
    {
        ;
    }
    else 
        throw(ErrorCodes(405));
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
    std::string file = pwd;
    int file_desc;

    if (cgi_request == false)
        file += "/html_files";
    if (URI == "/")
    {
        URI = "/index.html"; 
        content_type = "text/html";
    }
    file += URI;
    file_desc = open(file.c_str(), O_RDONLY);
    if (file_desc == -1)
    {
        code = 404; 
        throw(ErrorCodes(404));
    }
    pwd = file;
    read_and_store_file(file_desc);
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

    last_slash = URI.rfind('/');
    exstension = URI.rfind('.');
    if (exstension == std::string::npos && last_slash != std::string::npos)
    {
        content_type = std::string("application/octet-stream");
        return ;
    }
    for (int i = 0; static_cast<unsigned long>(i) < sizeof(file_exstensions) / sizeof(file_exstensions[0]); i++)
    {
        if (URI.compare(exstension + 1, std::string::npos, file_exstensions[i]) == 0)
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