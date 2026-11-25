#ifndef CHUNKED_HPP
# define CHUNKED_HPP

# include <string>
# include "HTTP.hpp"
# include "ErrorCodes.hpp"

class Chunked
{
    public:
    static bool check_chunked(t_header const *val, std::string const &other);
    static bool check_size_0(size_t index_start, std::string const &other);
    static std::string unchunk(std::string const &other, size_t index_start);
    private:
    static size_t parse_size(std::string const &line);
    Chunked();
    Chunked(Chunked const &other);
    Chunked &operator=(Chunked const &other);
    ~Chunked();
};

#endif
