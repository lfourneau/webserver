#include "ErrorCodes.hpp"

ErrorCodes::ErrorCodes() : _code(200), message(find_associated_value(this->getcode()))
{
}

ErrorCodes::ErrorCodes(int value) : _code(value), message(find_associated_value(this->getcode()))
{
}

ErrorCodes::ErrorCodes(ErrorCodes const &other) : ErrorCodes(other.getcode())
{
}

ErrorCodes &ErrorCodes::operator=(ErrorCodes const &other)
{
    return (*this);
}

ErrorCodes::~ErrorCodes()
{
}

std::string const ErrorCodes::getmessage() const
{
    return (this->message);
}

int const ErrorCodes::getcode() const 
{
    return (this->_code);
}

const char *ErrorCodes::what() const throw()
{
    return (this->getmessage().c_str());
}

std::string const ErrorCodes::find_associated_value(int const code)
{
    int const codes[] = {200, 201, 202, 203, 204, 205, 206, 207, 208,\
    300, 301, 302, 303, 304, 307, 308, \
    400, 401, 402, 403, 404, 405, 406, 407, 408, 409, 410, 411, 412,\
    413, 414, 415, 416, 417, 418, 421,\
    500, 501, 502, 503, 504, 505, 506, 507, 508, 999};
    std::string const messages[] = {"OK", "Created", "Accepted", "Non-Authoritative Information",\
    "No Content", "Reset Content", "Partial Content", "Multi-Status", "Already Reported", \
    "Multiple Choices", "Moved Permanently", "Found", "See Other", "Not Modified", "Temporary Redirect",\
    "Permanent Redirect", "Bad Request", "Unauthorized", "Payment Required", "Forbidden", "Not Found",\
    "Method Not Allowed", "Not Acceptable", "Proxy Authentication Required", "Request Timeout", \
    "Conflict", "Gone", "Length Required", "Precondition Failed", "Payload Too Large", "URI Too Long",\
    "Unsupported Media Type", "Range Not Satisfiable", "Expectation Failed", "I’m a Teapot", "Misdirected Request",\
    "Internal Server Error", "Not Implemented", "Bad Gateway", "Service Unavailable", "Gateway Timeout",
    "HTTP Version Not Supported", "Variant Also Negotiates", "Insufficient Storage", "Loop Detected",\
    "Invalid error code"};

    int i = 0;
    while (codes && codes[i] != 999)
    {
        if (codes[i] == code)
        {
            return (messages[i]);
        }
        i++;
    }
    return ("Invalid error code");
}



