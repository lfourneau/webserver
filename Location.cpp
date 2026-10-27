#include "Location.hpp"

Location::Location(IT &it, IT end)
{
    parse_location_block(it, end);
}

Location::Location(Location const &other)
{
    location = other.location;
}

Location &Location::operator=(Location const &other)
{
    if (this != &other)
    {
        location = other.location;
    }
    return (*this);
}

Location::~Location()
{
}

void    Location::parse_location_block(IT &it, IT end)
{
    if ((*it)[0] == '.')
    {
        parse_cgi(it, end);
        return ;
    }
    if ((*it)[0] == '/')
    {
        parse_location(it, end);
        return ;
    }
    throw(InvalidLocationBlock("location not a file or folder"));
}

void Location::parse_cgi(IT &it, IT end)
{
    if (!(*it == ".py" || *it == ".php"))
        throw (InvalidLocationBlock("not supported file type for cgi"));
    location.cgi_pass = true;
    ++it;
    if (*it != "{") 
        throw (InvalidLocationBlock("missing curly brackets after location block"));
    ++it;
    while (*it != "}")
    {
        extract_cgi_line(it, end);
    }
    if (*it != "}")
        throw(InvalidLocationBlock("Location blocks must end in }"));
    it++;
}

void Location::parse_location(IT &it, IT end)
{
    location.prefix = *it;
    ++it;

    if (*it++ != "{")
        throw(InvalidLocationBlock("curly bracket missing in location block"));
    std::string const accepted_rules[] = {"accepted_methods", "root", "index", "autoindex"};
    static void (Location::* const functions[])(IT &, IT) = {&Location::parse_accepted_methods,
        &Location::parse_root, \
        &Location::index_file, \
        &Location::parse_autoindex};
    while (it != end && *it != ";" && *it != "}")
    {
        for (int i = 0; i < 4; i++)
        {
            if (*it == accepted_rules[i])
            {
                ++it;
                (this->*functions[i])(it, end);
                ++it;
                break ;
            }
            if (i == 4)
                throw(InvalidLocationBlock("invalid request rule"));
        }
    }
    if (*it != "}")
        throw(InvalidLocationBlock("location blocks must have a closing }"));
    it++;
}

void Location::index_file(IT &it, IT end)
{
    if ((it + 1) == end || *(it + 1) != ";")
        throw(InvalidLocationBlock("Missing index_file argument or semi-colon"));
    location.default_file = *it;
    ++it;
}

void Location::extract_cgi_line(IT &it, IT end)
{
    std::string const accepted_rules[] = {"accepted_methods", "root", "fast_pass_cgi"};
    static void (Location::* const functions[])(IT &, IT) = {&Location::parse_accepted_methods,
        &Location::parse_root, \
        &Location::parse_cgi_exec};

    for (int i = 0; i < 3; i++)
    {
        if (*it == accepted_rules[i])
        {
            ++it;
            (this->*functions[i])(it, end);
            ++it;
            return ;
        }
    }
    throw(InvalidLocationBlock("invalid request rule"));
}

void    Location::parse_accepted_methods(IT &it, IT end)
{
    while (*it != ";" && it != end)
    {
        if (*it == "GET")
            location.get = true;
        else if (*it == "POST")
            location.post = true;
        else if (*it == "DELETE")
            location.del = true;
        else 
            throw(InvalidLocationBlock("HTTP method not supported by server"));
        ++it;
    }
    if (it == end)
        throw (InvalidLocationBlock("Location block is missing a semi colon"));
}

void    Location::parse_root(IT &it, IT end)
{
    if ((it + 1) == end || *(it + 1) != ";")
        throw(InvalidLocationBlock("Missing root argument or semi-colon"));
    location.file_directory = *it;
    ++it;
}

void    Location::parse_autoindex(IT &it, IT end)
{
    if ((it + 1) == end || *(it + 1) != ";")
        throw(InvalidLocationBlock("Missing autoindex argument or semi-colon"));
    if (!((*it == "on") || (*it == "off")))
        throw(InvalidLocationBlock("Only on or off is accepted for the autindex method"));
    if (*it == "on")
        location.autoindex = true;
    ++it;
}

void    Location::parse_cgi_exec(IT &it, IT end)
{
    if ((it + 1) == end || *(it + 1) != ";")
        throw(InvalidLocationBlock("Missing semi-colon or exec file"));   
    if (!(*it == "/usr/bin/php-cgi" || *it == "/usr/bin/python3"))
        throw(InvalidLocationBlock("only php or pyton files are accepted for php(/usr/bin/php-cgi) and python(/usr/bin/python3)"));
    if (*it == "/usr/bin/php-cgi")
        location.program_exec_cgi = "/usr/bin/php-cgi";
    else
        location.program_exec_cgi = "usr/bin/python3";
    ++it;
}

t_location const &Location::get_location() const
{
    return (this->location);
}