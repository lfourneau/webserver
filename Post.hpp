#ifndef POST_HPP
# define POST_HPP

# include <string>
# include "Location.hpp"
# include "HTTP.hpp"
# include "ExecuteRequest.hpp"

class Post
{
    public:
    static void post_file(t_execute *exec, t_header const *head, t_location const *loc);
    private:
    static std::string get_boundary(t_header const *head);
    static std::string get_upload_dir(t_execute const *exec);
    static bool parse_part(std::string const &part, std::string const &dir);
    static std::string get_filename(std::string const &part_headers);
    static void save_file(std::string const &path, std::string const &content);
    Post();
    Post(Post const &other);
    Post &operator=(Post const &other);
    ~Post();
};

#endif
