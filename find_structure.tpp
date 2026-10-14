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