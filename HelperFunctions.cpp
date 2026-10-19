#include "HelperFunctions.hpp"

HelperFunctions::HelperFunctions()
{
}

HelperFunctions::HelperFunctions(HelperFunctions const &other)
{
    (void)other;
}

HelperFunctions &HelperFunctions::operator=(HelperFunctions const &other)
{
    (void)other;
    return (*this);
}

HelperFunctions::~HelperFunctions()
{
    ;
}

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
