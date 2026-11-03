#ifndef POST_HPP
# define POST_HPP

# include "HTTP.hpp"
# include <iostream>
# include <string>
# include <vector>
# include <list>
# include <sstream>
# include "HelperFunctions.hpp"

class Post : public HTTP
{
    public:
    Post(HTTP const &other);
    Post(Post const &other);
    Post &operator=(Post const &other);
    virtual ~Post() throw();
    void request_post();
    private:
    void get_post_headers(std::istringstream &content);
    void get_file_content(std::istringstream &content);
    void save_to_upload();
    void get_form_data();
    bool check_multiform_data();
    int size_content;
    int max_char;
    std::string file_body;
    std::string seperator;
    std::map<std::string, std::string> body_headers;
    std::map<std::string, std::string> form_data;
};

# include "find_structure.tpp"
# include "read_until.tpp"

#endif