#include "ServerParser.hpp"

t_header *ServerParser::parse_input(std::string const &other, ssize_t const val)
{
    std::stringstream content_to_parse;
    content_to_parse.write(other.c_str(), val);

    t_header *parsed = new t_header;
    get_first_line(content_to_parse, parsed);
    check_cgi(parsed);
    parse_headers(content_to_parse, parsed);
    get_host_port(parsed);
    return (parsed);
}

void ServerParser::get_first_line(std::stringstream &stream, t_header *save)
{
    std::string line;

    getline(stream, line);
    std::istringstream line_s(line);
    std::string trash;
    if (!(line_s >> save->method >> save->URI >> save->protocol || !(line_s >> trash)))
    {
        throw(ErrorCodes(400));
    }
}

void    ServerParser::check_cgi(t_header *head)
{
    size_t index;

    // separe "/chemin?query" : URI garde le chemin, cgi_extension la query string
    index = head->URI.find('?');
    if (index == std::string::npos)
    {
        head->file_name = head->URI;
        return;
    }
    head->cgi_extension = head->URI.substr(index + 1);
    head->URI.erase(index);
    head->file_name = head->URI;
}

void    ServerParser::parse_headers(std::stringstream &val, t_header *header)
{
    std::string current_line;
    while (getline(val, current_line))
    {
        std::stringstream flux(current_line);
        std::string key;
        std::string value;

        getline(flux, key, ':');
        HelperFunctions::trim(key, " \r\n:");
        HelperFunctions::mini(key);
        getline(flux, value);
        HelperFunctions::trim(value, " \r\n:");
        header->headers[key] = value;
    }
}

void    ServerParser::get_host_port(t_header *header)
{
    std::string combo = header->headers["host"];
    if (combo == "")
    {
        throw(ErrorCodes(400));
    }
    std::istringstream exp(combo);
    getline(exp, header->host_name, ':');
    HelperFunctions::trim(header->host_name, " \r\n:");
    std::string port_str;
    getline(exp, port_str);
    HelperFunctions::trim(port_str, " \r\n:");
    header->host_port = transform_to_num<int>(port_str);
}