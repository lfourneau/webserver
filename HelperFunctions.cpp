#include "HelperFunctions.hpp"

std::string &HelperFunctions::trim_left(std::string &line, const char *char_to_trim)
{
    size_t  location;

    location = line.find_first_not_of(char_to_trim);
    if (location == std::string::npos)
    {
        line.clear();
        return (line);
    }
    line = line.substr(location);
    return (line);
}

std::string &HelperFunctions::trim_right(std::string &line, const char *char_to_trim)
{
    size_t  location;

    location = line.find_last_not_of(char_to_trim);
    if (line.empty() == true)
    {
        return line;
    }
    if (location == std::string::npos)
    {
        line.clear();
        return (line);
    }
    line = line.substr(0, location + 1);
    return (line);
}

std::string &HelperFunctions::trim(std::string &line, const char *char_to_trim)
{
    trim_right(line, char_to_trim);
    trim_left(line, char_to_trim);
    return(line);
}

bool    HelperFunctions::isblank(std::string const &line)
{
    std::string::const_iterator it;

    for (it = line.begin(); it != line.end(); it++)
    {
        if ((*it >= 9 && *it <= 13) || (*it == 32)) 
            continue;
        return (false);
    }
    return (true);
}

void    HelperFunctions::mini(std::string &val)
{
    for (unsigned int i = 0; i < val.size(); i++)
    {
        val[i] = static_cast<unsigned char>(tolower(val[i]));
    }
}
