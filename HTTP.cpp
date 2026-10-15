#include "HTTP.hpp"

HTTP::HTTP(int fd) : associated_fd(fd)
{
}

HTTP::HTTP(const HTTP &other)
{

}

HTTP    &HTTP::operator=(const HTTP &other)
{
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
    std::string line_one;
    std::stringstream content;

    //isolate the first line of the header
    content << request_content;
    std::getline(content, line_one, '\n');

    //isolate the request method, URI, protocol_version
    content.str("");
    content.clear();
    content << line_one;
    std::getline(content, request_method, ' ');
    std::getline(content, URI, ' ');
    std::getline(content, protocol_version, '\r');
} 

void HTTP::append_content(char *append_info, unsigned int total_read)
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