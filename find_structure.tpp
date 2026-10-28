#ifndef FIND_STRUCTURES_TPP
# define FIND_STRUCTURES_TPP

# include <string>
# include <istream>

template <typename T, typename V> T &find_value_vector(std::vector<T> &vec, \
    V (T::*member)(), V value)
{
    typename std::vector<T>::iterator it;

    for (it = vec.begin(); it != vec.end(); it++)
    {
        if (((*it).*member)() == value)
        {
                return (*it);
        }
    }
    return (*vec.end());
};

template <typename T, typename V> void delete_vector(std::vector<T> &vec, \
    V (T::*member)(), V value)
{
    typename std::vector<T>::iterator it;

    for (it = vec.begin(); it != vec.end(); it++)
    {
        if (((*it).*member)() == value)
        {
            vec.erase(it);
            return ;
        }
    }
};

template <typename T> void transform_to_num(std::string const &val, T &num)
{
    std::istringstream content(val);

    content >> num;
}

#endif