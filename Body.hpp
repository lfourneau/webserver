#ifndef BODY_HPP
# define BODY_HPP

# include <string>
# include "HTTP.hpp"
# include "ErrorCodes.hpp"

class Body
{
    public:
    static size_t get_content_length(t_header const *val);
    private:
    Body();
    Body(Body const &other);
    Body &operator=(Body const &other);
    ~Body();
};

#endif
