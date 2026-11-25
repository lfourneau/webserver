#include "Location.hpp"
#include <cstdlib>

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

//les location cgi commencent par des points
void    Location::parse_location_block(IT &it, IT end)
{
    if (it == end || it->empty())
        throw(InvalidLocationBlock("location needs a path"));
    if ((*it)[0] == '.' && (*it == ".py" || *it == ".php"))
    {
        parse_cgi(it, end);
        return ;
    }
    if ((*it)[0] == '/')
    {
        parse_location(it, end);
        return ;
    }
    throw(InvalidLocationBlock("location must start with / (folder) or . (CGI extension)"));
}

void Location::parse_cgi(IT &it, IT end)
{
    if (!(*it == ".py" || *it == ".php"))
        throw (InvalidLocationBlock("not supported file type for cgi"));
    location.cgi_pass = true;
    location.prefix = *it;
    ++it;
    if (it == end || *it != "{") 
        throw (InvalidLocationBlock("missing curly brackets after location block"));
    ++it;
    while (it != end && *it != "}")
    {
        extract_cgi_line(it, end);
    }
    if (it == end || *it != "}")
        throw(InvalidLocationBlock("Location blocks must end in }"));
    it++;
}

//on associe chaque ligne a l'information qu'elle precise
void Location::parse_location(IT &it, IT end)
{
    std::string const accepted_rules[] = {"accepted_methods", "root", "index", \
        "autoindex", "return", "upload_store"};
    static void (Location::* const functions[])(IT &, IT) = {&Location::parse_accepted_methods,
        &Location::parse_root, \
        &Location::index_file, \
        &Location::parse_autoindex, \
        &Location::parse_return, \
        &Location::parse_upload_store};
    int const nb_rules = 6;

    location.prefix = *it;
    ++it;
    if (it == end || *it++ != "{")
        throw(InvalidLocationBlock("curly bracket missing in location block"));
    while (it != end && *it != "}")
    {
        int i = 0;

        while (i < nb_rules && *it != accepted_rules[i])
            i++;
        if (i == nb_rules)
            throw(InvalidLocationBlock("invalid rule in location block: " + *it));
        ++it;
        (this->*functions[i])(it, end);
        ++it;
    }
    if (it == end || *it != "}")
        throw(InvalidLocationBlock("location blocks must have a closing }"));
    it++;
}

//sur nginx les codes pour les returns ont forcement des valeurs commencant en 300
void Location::parse_return(IT &it, IT end)
{
    std::string const &code = *it;

    if ((it + 1) == end || (it + 2) == end || *(it + 2) != ";")
        throw(InvalidLocationBlock("return needs a code, an URL and a semi-colon"));
    if (!(code == "301" || code == "302" || code == "303" || code == "307" || code == "308"))
        throw(InvalidLocationBlock("return code must be 301, 302, 303, 307 or 308"));
    location.redirect_code = std::atoi(code.c_str());
    location.redirect_url = *(it + 1);
    it = it + 2;
}

void Location::parse_upload_store(IT &it, IT end)
{
    if ((it + 1) == end || *(it + 1) != ";")
        throw(InvalidLocationBlock("upload_store needs one directory and a semi-colon"));
    location.upload_dir = *it;
    ++it;
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
    while (it != end && *it != ";")
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
        location.program_exec_cgi = "/usr/bin/python3";
    ++it;
}

t_location const &Location::get_location() const
{
    return (this->location);
}