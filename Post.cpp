#include "Post.hpp"

Post::Post(HTTP const &other) : HTTP(other)
{
    size_content = 0;
}

Post::Post(Post const &other) : HTTP(0)
{
    (void)other;
    return ;
}

Post    &Post::operator=(Post const &other)
{
   (void)other;
   return (*this);
}

Post::~Post()
{
}

void Post::request_post()
{
    size_t end_header = body.find("\r\n\r\n");
    std::stringstream   read_file(body.substr(0, end_header));
    std::stringstream   read_body(body.substr(end_header + 4, std::string::npos));

    get_post_headers(read_file);
    get_file_content(read_body);
    if (URI == "/upload")
    {
        save_to_upload();
        std::cout << file_body << std::endl;
    }
    
}

void    Post::save_to_upload()
{
    int fd;

    fd = open("./upload/test.txt", O_RDWR |  O_CREAT | O_TRUNC, 0644);
    write(fd, file_body.c_str(), file_body.size());
    close(fd);
}

void    Post::get_post_headers(std::stringstream &flux) 
{
    std::string line;
    std::string key;
    std::string value;
    max_char = atoi(header_lines.find("Content-Length")->second.c_str());
    size_t del;

    getline(flux, seperator);
    size_content += seperator.size() + 1;
    if (seperator[seperator.size() - 1] == '\r')
    {
        seperator.erase(seperator.size() - 1);
    }
    while (getline(flux, line))
    {
        if (line.empty() == true || line == "\r")
        {
            if (line.empty() == true)
                size_content += 1;
            size_content += 1;
            break;
        }
        size_content += line.size() + 1;
        if (line[line.size() - 1] == '\r')
        {
            line.erase(line.size() - 1);
        }
        if (size_content > max_char)
        {
            std::cout << "Too many characters in received string" << std::endl;
            return ;
        }
        del = line.find(':');
        key = line.substr(0, del);
        value = line.substr(del + 1, line.length());
        body_headers[key] = value;
    }
}

void   Post::get_file_content(std::stringstream &flux)
{
    std::string file_content;
    std::string delim;
    size_t find_end;

    file_content = flux.str(); 
    find_end = file_content.find(TransformToString() << "\r\n----" << file_body);
    file_body = file_content.substr(0, find_end);
    size_content += file_body.size();
    if (size_content > max_char)
    {
        std::cout << "invalid size of document" << std::endl;
    }
    delim = file_content.substr(find_end + 1, file_content.size());
    if (size_content + delim.size() == static_cast<size_t> (max_char))
    {
        std::cout << "the file is the right size" << std::endl;
    }

}