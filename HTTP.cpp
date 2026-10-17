#include "HTTP.hpp"

HTTP::HTTP(int fd) : associated_fd(fd)
{
}

HTTP::HTTP(const HTTP &other)
{
    this->associated_fd = other.associated_fd;
    this->request_method = other.request_method;
    this->URI = other.URI;
    this->protocol_version = other.protocol_version;
}

HTTP    &HTTP::operator=(const HTTP &other)
{
    if (this != &other)
    {
        this->associated_fd = other.associated_fd;
        this->request_method = other.request_method;
        this->URI = other.URI;
        this->protocol_version = other.protocol_version;
    }
    return (*this);
}

HTTP::~HTTP()
{

}

void HTTP::analyse_buffer()
{
    parse_input();
    close(associated_fd);
}

void    HTTP::parse_input()
{
    parse_initial_response_line();
    //parser le reste du header peut etre stocker le tout dans des paires;

    

} 

void HTTP::append_content(const char *append_info, unsigned int total_read)
{
    request_content.append(append_info, total_read);
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
    
}


void HTTP::parse_initial_response_line()
{
    std::stringstream flux;

    flux << request_content;

    std::getline(flux, request_content, '\n');
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
    std::s
    while ()
}