#ifndef TRANSFORM_NUM_HPP
# define TRANSFORM_NUM_HPP

# include <sstream>

template <typename T>
T transform_to_num(std::string const &val)
{
    std::istringstream str(val);
    T return_val;

    str >> return_val;
    return (return_val);
}

template <typename T>
void transform_to_num(std::string const &val, T &num)
{
    std::istringstream content(val);

    content >> num;
}

#endif