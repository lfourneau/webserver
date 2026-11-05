#ifndef GET_HPP
# define GET_HPP

# include "ExecuteRequest.hpp"
# include "HTTP.hpp"

class Get
{
    public:
    static std::string extract_output(t_execute *, t_header const *, t_location const *);
    static void read_file(t_execute *exec, std::string file_to_read);
    private:
    Get();
    Get(Get const &other);
    Get &operator=(Get const &other);
    ~Get();
};


#endif