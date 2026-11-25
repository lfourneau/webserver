#ifndef HELPERFUNCTIONS_HPP
# define HELPERFUNCTIONS_HPP

# include <string>

class   HelperFunctions
{
    public:

        static std::string &trim_right(std::string &line, const char *char_to_trim);
        static std::string &trim_left(std::string &line, const char *char_to_trim);
        static std::string &trim(std::string &line, const char *char_to_trim);
        static bool isblank(std::string const &line);
        static void mini(std::string &val);
        static void maxi(std::string &val);
        static std::string ft_itoa(long val);
    private:
        HelperFunctions();
        HelperFunctions(HelperFunctions const &other);
        HelperFunctions &operator=(HelperFunctions const &other);
        ~HelperFunctions();
};

#endif