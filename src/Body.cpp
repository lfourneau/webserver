#include "Body.hpp"
#include <cstdlib>

size_t Body::get_content_length(t_header const *val)
{
    std::map<std::string, std::string>::const_iterator it = \
    val->headers.find("content-length");
    char *end;
    unsigned long size;

    if (it == val->headers.end())
        return (0);
    if (it->second.empty() || it->second.find_first_not_of("0123456789") != std::string::npos)
        throw ErrorCodes(400);
    size = std::strtoul(it->second.c_str(), &end, 10);
    if (*end != '\0')
        throw ErrorCodes(400);
    return (static_cast<size_t>(size));
}
