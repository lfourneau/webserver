#include "Chunked.hpp"
#include <cstdlib>

bool Chunked::check_chunked(t_header const *val, std::string const &other)
{
    std::map<std::string, std::string>::const_iterator it = \
    val->headers.find("transfer-encoding");

    if (it == val->headers.end())
        return (true);
    if (it->second != "chunked")
        throw ErrorCodes(501);

    size_t end_header = other.find("\r\n\r\n");
    if (end_header == std::string::npos)
        return (false);
    return (check_size_0(end_header + 4, other));
}

bool Chunked::check_size_0(size_t index_start, std::string const &other)
{
    size_t pos = index_start;
    size_t eol;
    size_t size;

    while (true)
    {
        eol = other.find("\r\n", pos);
        if (eol == std::string::npos)
            return (false);
        size = parse_size(other.substr(pos, eol - pos));
        pos = eol + 2;
        if (size == 0)
        {
            if (other.size() < pos + 2)
                return (false);
            if (other.compare(pos, 2, "\r\n") != 0)
                throw ErrorCodes(400);
            return (true);
        }
        if (other.size() < pos + size + 2)
            return (false);
        if (other.compare(pos + size, 2, "\r\n") != 0)
            throw ErrorCodes(400);
        pos += size + 2;
    }
}

std::string Chunked::unchunk(std::string const &other, size_t index_start)
{
    std::string body;
    size_t pos = index_start;
    size_t eol;
    size_t size;

    while (true)
    {
        eol = other.find("\r\n", pos);
        size = parse_size(other.substr(pos, eol - pos));
        pos = eol + 2;
        if (size == 0)
            break;
        body.append(other, pos, size);
        pos += size + 2;
    }
    return (body);
}

size_t Chunked::parse_size(std::string const &line)
{
    std::string hex = line.substr(0, line.find(';'));
    char *end;
    unsigned long size;

    if (hex.empty() || hex.find_first_not_of("0123456789abcdefABCDEF") != std::string::npos)
        throw ErrorCodes(400);
    size = std::strtoul(hex.c_str(), &end, 16);
    if (*end != '\0')
        throw ErrorCodes(400);
    return (static_cast<size_t>(size));
}
