#ifndef ERROR_CODES_HPP
# define ERROR_CODES_HPP

# include <exception>
# include <string>

class ErrorCodes : public std::exception
{
    public:
    ErrorCodes();
    ErrorCodes(int value);
    ErrorCodes(ErrorCodes const &other);
    ErrorCodes &operator=(ErrorCodes const &other);
    ~ErrorCodes();
    virtual const char *what() const throw();
    int const getcode() const;
    std::string const getmessage() const;
    private:
    const int _code;
    const std::string message; 
    const std::string find_associated_value(int const code);

};

#endif