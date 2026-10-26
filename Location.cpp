#include "Location.hpp"

Location::Location(IT &it, IT end)
{
}

Location::Location(Location const &other)
{
    (void)other;
}

Location &Location::operator=(Location const &other)
{
    (void)other;
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
    ++it;
    if (*it != "{") 
        throw (InvalidLocationBlock("missing curly brackets after location block"));
    ++it;
    while (*it != "}")
    {
        extract_cgi_line(it, end);
    }
}

void Location::parse_location(IT &it, IT end)
{
    this->file_redirect = *it;
    ++it;

    std::string const accepted_rules[] = {"accepted_methods", "root", "index", "autoindex"};
    static void (Location::* const functions[])(IT &, IT) = {&Location::parse_accepted_methods,
        &Location::parse_root, \
        &Location::index_file, \
        &Location::parse_autoindex};

    for (int i = 0; i < 4; i++)
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

void Location::index_file(IT &it, IT end)
{
    if ((it + 1) == end || *(it + 1) != ";")
        throw(InvalidLocationBlock("Missing index_file argument or semi-colon"));
    this->default_file = *it;
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
            get = true;
        else if (*it == "POST")
            post = true;
        else if (*it == "DELETE")
            del = true;
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
    this->file_directory = *it;
    ++it;
}

void    Location::parse_autoindex(IT &it, IT end)
{
    if ((it + 1) == end || *(it + 1) != ";")
        throw(InvalidLocationBlock("Missing autoindex argument or semi-colon"));
    if (!(*it == "on") || (*it == "off"))
        throw(InvalidLocationBlock("Only on or off is accepted for the autindex method"));
    if (*it == "on")
        this->autoindex = true;
    ++it;
}

void    Location::parse_cgi_exec(IT &it, IT end)
{
    if ((it + 1) == end || *(it + 1) != ";")
        throw(InvalidLocationBlock("Missing semi-colon or exec file"));   
    if (!(*it == "/usr/bin/php-cgi" || *it == "usr/bin/python3"))
        throw(InvalidLocationBlock("only php or pyton files are accepted for php(/usr/bin/php-cgi) and python(usr/bin/python3)"));
    if (*it == "/usr/bin/php-cgi")
        this->program_exec_cgi = "/usr/bin/php-cgi";
    else
        this->program_exec_cgi = "usr/bin/python3";
    ++it;
}

void Location::parse_location(std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end)
{
    
}