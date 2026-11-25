#ifndef DELETE_HPP
# define DELETE_HPP

# include "Location.hpp"
# include "HTTP.hpp"
# include "ExecuteRequest.hpp"

class Delete
{
    public:
    static void delete_file(t_execute *exec, t_header const *head, t_location const *loc);
    private:
    Delete();
    Delete(Delete const &other);
    Delete &operator=(Delete const &other);
    ~Delete();
};

#endif
