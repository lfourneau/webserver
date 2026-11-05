#include "Post.hpp"

Post::Post(HTTP const &other) : HTTP(other)
{
    size_content = 0;
}

Post::Post(Post const &other) : HTTP(other)
{
    (void)other;
    return ;
}

Post    &Post::operator=(Post const &other)
{
   (void)other;
   return (*this);
}

Post::~Post() throw()
{
}

void Post::request_post()
{
    size_t end_header = body.find("\r\n\r\n");
    std::istringstream   read_file(body.substr(0, end_header));
    std::istringstream   read_body(body.substr(end_header + 4, std::string::npos));

    get_post_headers(read_file);
    get_form_data();
    get_file_content(read_body);
    save_to_upload();
    std::cout << file_body << std::endl;
    
}

void    Post::save_to_upload()
{
    int fd;
    std::string filename;

    if (loc.file_directory != "")
        filename = TransformToString() << loc.file_directory << loc.prefix << form_data["filename"];
    else 
        filename = TransformToString() << bloc.server_repo << loc.prefix << form_data["filename"];
    fd = open(filename.c_str(), O_RDWR |  O_CREAT | O_TRUNC, 0644);
    write(fd, file_body.c_str(), file_body.size());
    close(fd);
}

void    Post::get_post_headers(std::istringstream &flux) 
{
    std::string line;
    std::string key;
    std::string value;
    transform_to_num(info_request["Content-Length"], max_char);
    size_t del;

    //if (check_multiform_data())
    //{
    //    read_until(flux, seperator);
    //    size_content += seperator.size() + 1;
    //    if (seperator[seperator.size() - 1] == '\r')
    //        seperator.erase(seperator.size() - 1);
    //}
    while (getline(flux, line))
    {
        if (line.empty() == true || line == "\r")
        {
            if (line.empty() == true)
                size_content += 1;
            size_content += 1;
            break;
        }
        size_content += line.size();
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

bool    Post::check_multiform_data()
{
    std::string section;
    std::istringstream get_content_type(this->info_request["Content-Type"]);
    bool contains_multiform = false;
    
    while (get_content_type >> section) 
    {
        if (section[section.size() - 1] == ';')
        {
            section.erase(section.size() - 1, std::string::npos);
        }
        if (section == "multipart/form-data")
        {
            contains_multiform = true;
        }
        if (contains_multiform == true)
        {
            if (!section.compare(0, 9,"boundary="))
            {
                std::string sep(section.c_str() + 9, section.size() - 9);
                seperator = sep; 
            }
        }
    }
    return (contains_multiform);
}

void    Post::get_form_data()
{
    std::istringstream content(body_headers["Content-Disposition"]);
    std::string extract_info;

    content >> extract_info;
    extract_info = HelperFunctions::trim(extract_info, ";");
    if (extract_info != "form-data")
        return ;
    while (content >> extract_info)
    {
        size_t colon_location; 
        std::string key;
        std::string value;

        colon_location = extract_info.find("=");
        key = extract_info.substr(0, colon_location);
        value =  extract_info.substr(colon_location + 1, extract_info.length());
        key = HelperFunctions::trim(key, "\";");
        value = HelperFunctions::trim(value, "\";");
        form_data[key] = value ;
    }
}

void   Post::get_file_content(std::istringstream &flux)
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