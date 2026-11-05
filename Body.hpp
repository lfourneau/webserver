#ifndef BODY_HPP 
# define BODY_HPP

# include <string> 
# include "HTTP.hpp"
# include "ErrorCodes.hpp"

class Body
{
    public:
    static void check_body_size(t_header const *val, std::string const &cont);
    static void check_length_body(std::string const &other, size_t index, int length);
    private:
    Body();
    Body(Body const &other);
    Body &operator=(Body const &other);
    ~Body();
};

#endif