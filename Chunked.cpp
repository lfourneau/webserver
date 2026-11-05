#include "Chunked.hpp"

bool Chunked::check_chunked(t_header const * val, std::string const &other)
{
    if (val->headers.count("transfer-encoding"))
    {
        std::map<std::string, std::string>::const_iterator it = \
        val->headers.find("transfer-encoding");
        if (it == val->headers.end())
            return (false);
        if (it->second != "chunked")
            return (false);
    }
    else
        return (true);
    
    size_t end_header = other.find("\r\n\r\n");
    end_header += 4;

    size_t end_chunk = other.find("0\r\n\r\n", end_header);
    if (end_chunk == std::string::npos)
        return (false);
    return (check_size_0(end_header, other));
     
}

bool Chunked::check_size_0(size_t index_start, std::string const &other)
{
    std::istringstream content(other.substr(index_start, std::string::npos));
    size_t size_chunk;
    std::string associated_val;

    while (true) 
    {
        if (content >> size_chunk)
        {
            if (size_chunk == 0)
            {
                if (!(content >> associated_val))
                {
                    throw ErrorCodes(400);
                }
                else
                    return (true);
            }
        }
        else
            return (false);
        if (content >> associated_val)
        {
            if (associated_val.size() != size_chunk)
            {
                throw ErrorCodes(400);
            }
        }
        else
            return (false);
    }
}