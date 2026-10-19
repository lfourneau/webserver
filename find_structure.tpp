#ifndef FIND_STRUCTURES_TPP
# define FIND_STRUCTURES_TPP

# include <string>

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

template <typename T, typename Y> char *to_c_str(T one, Y two)
{
    std::stringstream content;
    std::string result;

    content << one << two;
    result << content;
    return (result.c_str());
}

#endif