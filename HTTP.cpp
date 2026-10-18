#include "HTTP.hpp"

HTTP::HTTP(int fd) : associated_fd(fd), pwd(PATH_FILES_DIRECTORY), code(200)
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

void HTTP::analyse_buffer()
{
    try
    {
        parse_input();
        send_response();
    }
    catch(const std::exception& e)
    {
        send_response(e.what());
    }
    close(associated_fd);
}

void    HTTP::parse_input()
{
    parse_initial_response_line();
    parse_header_lines();
} 

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

int HTTP::get_client_fd()
{
    return (this->associated_fd);
}

void HTTP::print_content()
{
    std::cout << this->request_content;
}

void HTTP::send_to_client()
{
    if (request_method == "GET")
        std::cout << "Get method found" << std::endl;
}


void HTTP::parse_initial_response_line()
{
    std::stringstream flux;
    std::string first_line;

    flux << this->request_content;
    std::getline(flux, first_line, '\n');
    flux.str("");
    flux.clear();
    flux << first_line;
    //cette synthaxe permet de lire la ligne en ignorant les moments ou il peut y avoir
    //des characeteres d'espace redondants
    flux >> request_method >> URI >> protocol_version;

    //on utilise une version plus acceptante de http donc il faut faire en sorte que le \r ne soit
    //pas accepte
    request_method = HelperFunctions::trim(request_method, " \n\r");
    URI = HelperFunctions::trim(URI, " \n\r");
    protocol_version = HelperFunctions::trim(protocol_version, " \n\r");
    
    //verifier que les informations transmises soient interpretables par le serveur
}

void HTTP::parse_header_lines()
{
    std::stringstream all_lines;
    std::stringstream line_flux;
    std::string current_line;
    std::string key;
    std::string value;

    all_lines << this->request_content;
    //line to delete the first line of the header
    std::getline(all_lines, current_line, '\n');
    while (true)
    {
        std::pair<std::string, std::string> new_pair;
        std::getline(all_lines, current_line, '\n');
        //attention un header peut techniquement commencer par un espace dans quel cas
        // ce n'est pas du contenu supplementaire mais qqch ratache a la requete d'avant;
        if (current_line.empty() == false && (current_line[0] == ' ' || current_line[0] == '\t'))
        {
            std::cerr << "This file does not support header lines on multiple lines";
            continue;
        }
        if (current_line.empty() == true || current_line == "\r" || current_line == "")
            break;

        line_flux << current_line;
        //attention cela ne marche que dans le cadre de paires
        //si c'est une liste il faut separer avec le ":"
        line_flux >> key >> value;
        //ici il faudrait peut etre verifier la presence de ce charactere :
        key = HelperFunctions::trim_right(key, ":");
        value = HelperFunctions::trim(value, "\r ");
        if (value.empty() == true || key.empty() == true)
            throw(ErrorInHeader());
        new_pair.first = key;
        new_pair.second = value; 
        this->header_lines.push_back(new_pair);
    }
}

void    HTTP::send_response()
{
    std::string file = pwd;
    int file_desc;

    if (this->request_method == "GET")
    {
        if (URI == "/")
            URI = "/index.html"; 
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

void    HTTP::send_response(const char *error) 
{
    std::string str_error = error;
    int fd;
    
    this->pwd.append("/error_page.html");
    if (str_error == "404 Not Found")
    {
        fd = open(this->pwd.c_str(), O_RDONLY);
        //ici peut etre verifier que ca ne renvoie pas -1
        read_and_store_file(fd);
    }
}

void    HTTP::read_and_store_file(int fd)
{
    char    buffer[4096];
    std::string file;
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
        file.append(buffer, current_buffer);
        bzero(buffer, 4096);
    }
    GETResponse(file, file.size());
}

void    HTTP::GETResponse(std::string const &content_file, size_t size_file)
{
    std::string file_type;
    std::string return_carriage = "\r\n";

    std::string first_line;
    
    first_line = output_first_line();
    send(this->associated_fd, first_line.c_str(), first_line.size(), 0);

    file_type = HTTP::determine_extension(pwd);
    send(this->associated_fd, file_type.c_str(), file_type.size(), 0);

    std::cout << file_type;

    std::string cont = content_length(size_file);
    send(this->associated_fd, cont.c_str(), cont.size(), 0);
    std::cout << cont;

    send(this->associated_fd, return_carriage.c_str(), return_carriage.size(), 0);
    std::cout << "\r\n";

    send(this->associated_fd, content_file.c_str(), size_file, 0);
    std::cout << content_file << std::endl;
}

std::string const  HTTP::determine_extension(std::string const &other) const
{
    size_t last_slash;
    size_t exstension;
    std::string file_exstensions[] = {"html", "htm", "css", "js", "txt", "json", "png", "jpg", "jpeg", "gif", "svg", "ico", "pdf"};
    std::string file_type[] = {"text/html", "text/html", "text/css", "text/javascript", "text/plain", "application/json", \
    "image/png", "image/jpeg", "image/jpeg", "image/gif", "image/svg+xml", "image/x-ico", "application/pdf" };

    last_slash = other.rfind('/');
    exstension = other.rfind('.');
    if (exstension == std::string::npos && last_slash != std::string::npos)
        return ("Content-Type: application/octet-stream\r\n");
    for (int i = 0; static_cast<unsigned long>(i) < sizeof(file_exstensions) / sizeof(file_exstensions[0]); i++)
    {
        if (other.compare(exstension + 1, std::string::npos, file_exstensions[i]) == 0)
            return ("Content-Type: " + file_type[i] + "\r\n");
    }
    return ("Content-Type: error\r\n");
    //ici il faut gerer une erreure
    
}

std::string const   HTTP::content_length(size_t &value) const
{
    std::ostringstream content;
    
    content << "Content-Length: " << value << "\r\n";
    return (content.str());
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