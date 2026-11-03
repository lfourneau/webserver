#ifndef READ_UNTIL_HPP
# define READ_UNTIL_HPP

# include <string>

template <typename T>
bool    read_until(T &stream, std::string const &find)
{
    unsigned int i = 0;
    unsigned int j = find.size();
    char c;

    while (stream.get(c))
    {
        if (find[i] != c)
        {
            i = 0;
        }
        if (i == j)
            return (true);
        i++;
    }
    return (false);
}

#endif