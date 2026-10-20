#ifndef HELPERFUNCTIONS_HPP
# define HELPERFUNCTIONS_HPP

# include <string>

class   HelperFunctions
{
    public:
        HelperFunctions();
        HelperFunctions(HelperFunctions const &other);
        HelperFunctions &operator=(HelperFunctions const &other);
        ~HelperFunctions();
        static std::string &trim_right(std::string &line, const char *char_to_trim);
        static std::string &trim_left(std::string &line, const char *char_to_trim);
        static std::string &trim(std::string &line, const char *char_to_trim);
        static bool isblank(std::string &line);
    private:
};

#endif