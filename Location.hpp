#ifndef LOCATION_HPP
# define LOCATION_HPP

# include <cstring>
# include <string>
# include <vector>
# include <exception>

typedef struct s_location
{
    bool    get;
    bool    post;
    bool    del;
    std::string file_directory;
    std::string prefix;
    std::string default_file;
    bool    autoindex;
    bool    cgi_pass;
    std::string program_exec_cgi;

    s_location() : get(false), post(false), del(false), file_directory(),\
    prefix(), default_file(), autoindex(false), cgi_pass(false),\
    program_exec_cgi() {};
} t_location;

class   Location
{
    public:
    typedef std::vector<std::string>::iterator IT;
    Location(IT &it, IT end);
    Location(Location const &other);
    Location &operator=(Location const &other);
    ~Location();
    class InvalidLocationBlock : public std::exception
    {
        public:
        InvalidLocationBlock(std::string reason) : reason(reason) {};
        ~InvalidLocationBlock() {};
        virtual const char * what() const throw()
        {
            return (this->reason.c_str());
        }
        private:
        std::string reason;
    };
    t_location const &get_location() const;

    private:
    t_location location;

    void    parse_location_block(IT &it, IT end);
    void    parse_cgi(IT &it, IT end);
    void    extract_cgi_line(IT &it, IT end);
    void    parse_location(IT &it, IT end);
    //functions to parse location blocks
    void    parse_accepted_methods(IT &it, IT end);
    void    parse_root(IT &it, IT end);
    void    parse_autoindex(IT &it, IT end);
    void    parse_cgi_exec(IT &it, IT end);
    void    index_file(IT &it, IT end);
};

#endif