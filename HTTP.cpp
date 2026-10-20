#include "HTTP.hpp"
#include "CGI.hpp"

//Les constructeurs

HTTP::HTTP(int fd) : associated_fd(fd), pwd(PATH_FILES_DIRECTORY), cgi_request(false), code(200)
{
}

HTTP::HTTP(const HTTP &other)
{
    this->associated_fd = other.associated_fd;
    this->request_method = other.request_method;
    this->URI = other.URI;
    this->protocol_version = other.protocol_version;
    this->request_content = other.request_content;
    this->header_lines = other.header_lines;
    this->pwd = other.pwd;
    this->code = other.code;
}

HTTP    &HTTP::operator=(const HTTP &other)
{
    if (this != &other)
    {
        this->associated_fd = other.associated_fd;
        this->request_method = other.request_method;
        this->URI = other.URI;
        this->protocol_version = other.protocol_version;
        this->request_content = other.request_content;
        this->header_lines = other.header_lines;
        this->pwd = other.pwd;
        this->code = other.code;
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
    std::cout << this->request_content;
}

void    HTTP::get_value_host()
{
    std::vector<std::pair <std::string, std::string> >::const_iterator it;

    for (it = header_lines.begin(); it != header_lines.end(); it++)
    {
        if (it->first == "Host")
        {
            host_port = it->second;
            return;
        }
    }
    std::cout << "Error with the host header line" << std::endl;
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
    std::stringstream flux;

    flux << this->request_content;
    try
    {
        parse_initial_response_line(flux);
        parse_header_lines(flux);
        get_value_host();
        read_file();
        GETResponse(content);
    }
    catch(const std::exception& e)
    {
        read_error_file(e.what());
        GETResponse(content);
    }
    close(associated_fd);
}

void HTTP::parse_initial_response_line(std::stringstream &flux)
{
    std::string first_line;

    if (!getline(flux, first_line))
    {
        std::cout << "The has been an error getting the line";
    }
    
    std::istringstream input(first_line);
    std::string extra_values;

    // la synthaxe renvoie vrai si on a pu lire et faux dans le cas contraire
    if (!(input >> request_method >> URI >> protocol_version) || (input >> extra_values))
    {
        std::cout << "The header is not valid" << std::endl;
    }
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
            throw(ErrorInHeader());
        this->header_lines.push_back(std::make_pair(key, value));
    }
}

void    HTTP::add_to_previous_pair(std::string &content)
{

    if (this->header_lines.empty() == true)
    {
        std::cout << "there is an error with the request";
        return;
    }

    std::pair<std::string, std::string> &it = this->header_lines.back();
    it.second.append(content);
}

// ici on lit et on stocke le contenu du fichier dans une string
void    HTTP::read_file()
{
    std::string file = pwd;
    int file_desc;

    check_cgi_request();
    if (this->request_method == "GET")
    {
        if (URI == "/")
            URI = "/index.html"; 
        determine_extension();
        file += URI;
        file_desc = open(file.c_str(), O_RDONLY);
        if (file_desc == -1)
        {
            code = 404; 
            throw(FileNotFound());
        }
    }
    pwd = file;
    read_and_store_file(file_desc);
}

void    HTTP::check_cgi_request()
{
    if (URI.find("?") == std::string::npos)
        return;

    std::stringstream line;
    line << URI;
    URI = "";
    URI.clear();
    getline(line, URI, '?');
    getline(line, query_string);
    cgi_request = true;
}

void    HTTP::determine_extension()
{
    size_t last_slash;
    size_t exstension;
    std::string file_exstensions[] = {"html", "htm", "css", "js", "txt", "json", "png", "jpg", "jpeg", "gif", "svg", "ico", "pdf"};
    std::string file_type[] = {"text/html", "text/html", "text/css", "text/javascript", "text/plain", "application/json", \
    "image/png", "image/jpeg", "image/jpeg", "image/gif", "image/svg+xml", "image/x-ico", "application/pdf" };

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
            return ;
        }
    }
    //attention ne fonctionne pas si c'est un fichier .php
    content_type = std::string("text/html");
    
}

void    HTTP::read_error_file(const char *error) 
{
    int fd;
    (void)error;

    this->pwd = "";
    this->pwd = TransformToString() << PATH_FILES_DIRECTORY << "/error_page.html";
    content_type = "text/html";
    fd = open(this->pwd.c_str(), O_RDONLY);
        //ici peut etre verifier que ca ne renvoie pas -1
    read_and_store_file(fd);
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

void    HTTP::GETResponse(std::string const &content_file)
{
    std::ostringstream output_message;
    std::string yoyo;
    (void) content_file;

    if (cgi_request)
    {
        CGI     send_to_cgi(*this);

        std::cout << "CGI instance created" << std::endl;

        content = send_to_cgi.get_output();
        output_message << output_first_line() << "Content-Type: text/html" << "\r\n" \
        << "Content-Length: " << content.size() << "\r\n\r\n" << content;
        yoyo = output_message.str();
        send(this->associated_fd, content.c_str(), content.size(), 0);

        return;
    }
    output_message << output_first_line() << "Content-Type:" << content_type << "\r\n" \
    << "Content-Length: " << content.size() << "\r\n\r\n" << content;
    yoyo = output_message.str();
    std::cout << yoyo << "\n\n" << content << std::endl;
    send(this->associated_fd, yoyo.c_str(), yoyo.size(), 0);

    return;
}

std::string const HTTP::output_first_line()
{
    int supported_codes[] = {200, 404};
    std::string associated_messages[] = {"OK", "Not Found"};
    std::ostringstream exit_string;

    exit_string << "HTTP/1.0 "  << code << " ";

    for (unsigned long i = 0; i < sizeof(supported_codes) / sizeof(supported_codes[0]); i++)
    {
        if (code == supported_codes[i])
            exit_string << associated_messages[i];
    }
    exit_string << "\r\n";
    return (exit_string.str());
}