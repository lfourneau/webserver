#include "Body.hpp"

void    Body::check_body_size(t_header const *val, std::string const &other)
{
    int  size;
    std::string check_format;

    if (val->headers.count("content-length"))
    {
        std::map<std::string, std::string>::const_iterator it = \
        val->headers.find("content-length");
        std::istringstream trans(it->second);
        if (trans >> size || !(trans >> check_format) || size < 0)
            throw(ErrorCodes(400));
    }
    else 
        return;
    size_t index_return = other.find("\r\n\r\n");
    if (index_return == std::string::npos)
        throw(ErrorCodes(400));
    check_length_body(other, index_return + 4, size);
}

void    Body::check_length_body(std::string const &other, size_t index, int length)
{
    std::string body;
    size_t end;

    body = other.substr(index, std::string::npos);
    end = body.find("\r\n\r\n");
    if (end - index != static_cast<size_t>(length))
        throw(ErrorCodes(400));
}