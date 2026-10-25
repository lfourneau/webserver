#ifndef DELETE_HPP
# define DELETE_HPP

# include "HTTP.hpp"
# include "ErrorCodes.hpp"
# include <cstdio>

class Delete : public HTTP
{
    public:
    Delete(HTTP const &other);
    Delete(Delete const &other);
    Delete &operator=(Delete const &other);
    ~Delete();
    void    request_delete();

};


#endif