#include "ServerParser.hpp"

t_header *ServerParser::parse_input(std::string const &other, ssize_t const val)
{
    std::stringstream content_to_parse;
    content_to_parse.write(other.c_str(), val);

    t_header *parsed = new t_header();
    // une requete invalide lance une exception : on libere avant de la relancer
    try
    {
        get_first_line(content_to_parse, parsed);
        check_cgi(parsed);
        decode_uri(parsed);
        parse_headers(content_to_parse, parsed);
        get_host_port(parsed);
    }
    catch (...)
    {
        delete parsed;
        throw ;
    }
    return (parsed);
}

/*"METHODE /chemin HTTP/1.1" : exactement trois elements.
methode en majuscules (une methode inconnue mais bien formee donne 501
plus tard), URI qui commence par '/', version HTTP/1.0 ou HTTP/1.1 (505 sinon)*/
void ServerParser::get_first_line(std::stringstream &stream, t_header *save)
{
    std::string line;
    std::string trash;

    getline(stream, line);
    std::istringstream line_s(line);
    if (!(line_s >> save->method >> save->URI >> save->protocol) || (line_s >> trash))
        throw(ErrorCodes(400));
    if (save->method.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ") != std::string::npos)
        throw(ErrorCodes(400));
    if (save->URI[0] != '/')
        throw(ErrorCodes(400));
    if (save->protocol.compare(0, 5, "HTTP/") != 0)
        throw(ErrorCodes(400));
    if (save->protocol != "HTTP/1.1" && save->protocol != "HTTP/1.0")
        throw(ErrorCodes(505));
}

void    ServerParser::check_cgi(t_header *head)
{
    size_t index;

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

static int hex_value(char c)
{
    if (c >= '0' && c <= '9')
        return (c - '0');
    if (c >= 'a' && c <= 'f')
        return (c - 'a' + 10);
    if (c >= 'A' && c <= 'F')
        return (c - 'A' + 10);
    return (-1);
}

//special characters can be encoded with %val to symbolise invalid notations (ie %20 for space)
//if a file contains a %20 it needs to be interpreted as a space
//check if requested path is invalid because starting with dots (move up one level)
void    ServerParser::decode_uri(t_header *head)
{
    std::string decoded;
    std::string const &uri = head->URI;

    for (size_t i = 0; i < uri.size(); i++)
    {
        if (uri[i] == '%')
        {
            if (i + 2 >= uri.size() || hex_value(uri[i + 1]) < 0 || hex_value(uri[i + 2]) < 0)
                throw(ErrorCodes(400));
            char c = static_cast<char>(hex_value(uri[i + 1]) * 16 + hex_value(uri[i + 2]));
            if (c == '\0')
                throw(ErrorCodes(400));
            decoded += c;
            i += 2;
        }
        else
            decoded += uri[i];
    }
    if (decoded.find("/../") != std::string::npos
        || (decoded.size() >= 3 && decoded.compare(decoded.size() - 3, 3, "/..") == 0))
        throw(ErrorCodes(400));
    head->URI = decoded;
    head->file_name = decoded;
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

//host is mandatory for http1.0
void    ServerParser::get_host_port(t_header *header)
{
    std::string combo = header->headers["host"];
    if (combo == "")
    {
        if (header->protocol == "HTTP/1.1")
            throw(ErrorCodes(400));
        return ;
    }
    std::istringstream exp(combo);
    getline(exp, header->host_name, ':');
    HelperFunctions::trim(header->host_name, " \r\n:");
    std::string port_str;
    getline(exp, port_str);
    HelperFunctions::trim(port_str, " \r\n:");
    if (port_str.empty())
        return ;
    if (port_str.find_first_not_of("0123456789") != std::string::npos || port_str.size() > 5)
        throw(ErrorCodes(400));
    header->host_port = transform_to_num<int>(port_str);
}