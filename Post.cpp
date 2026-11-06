#include "Post.hpp"
#include "ErrorCodes.hpp"
#include <map>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

void    Post::post_file(t_execute *exec, t_header const *head, t_location const *loc)
{
    std::string const &body = head->body_request;
    std::string delim;
    std::string dir;
    size_t pos;
    size_t next;
    int saved;

    if (loc->cgi_pass == true)
        return ;
    delim = "--" + get_boundary(head);
    dir = get_upload_dir(exec);
    if (body.compare(0, delim.size(), delim) != 0)
        throw ErrorCodes(400);
    pos = delim.size();
    saved = 0;
    while (true)
    {
        if (body.compare(pos, 2, "--") == 0)
            break;
        if (body.compare(pos, 2, "\r\n") != 0)
            throw ErrorCodes(400);
        pos += 2;
        next = body.find("\r\n" + delim, pos);
        if (next == std::string::npos)
            throw ErrorCodes(400);
        if (parse_part(body.substr(pos, next - pos), dir))
            saved++;
        pos = next + 2 + delim.size();
    }
    if (saved == 0)
        throw ErrorCodes(400);
    exec->code = 201;
}

std::string Post::get_boundary(t_header const *head)
{
    std::map<std::string, std::string>::const_iterator it = \
    head->headers.find("content-type");
    std::string boundary;
    size_t pos;

    if (it == head->headers.end() || it->second.compare(0, 19, "multipart/form-data") != 0)
        throw ErrorCodes(415);
    pos = it->second.find("boundary=");
    if (pos == std::string::npos)
        throw ErrorCodes(400);
    boundary = it->second.substr(pos + 9);
    pos = boundary.find(';');
    if (pos != std::string::npos)
        boundary.erase(pos);
    if (boundary.size() >= 2 && boundary[0] == '"' && boundary[boundary.size() - 1] == '"')
        boundary = boundary.substr(1, boundary.size() - 2);
    if (boundary.empty())
        throw ErrorCodes(400);
    return (boundary);
}

std::string Post::get_upload_dir(t_execute const *exec)
{
    struct stat info;
    std::string dir = exec->filename;
    size_t slash;

    if (stat(dir.c_str(), &info) == -1 || !S_ISDIR(info.st_mode))
    {
        slash = dir.rfind('/');
        if (slash == std::string::npos)
            dir = ".";
        else
            dir.erase(slash);
        if (stat(dir.c_str(), &info) == -1 || !S_ISDIR(info.st_mode))
            throw ErrorCodes(404);
    }
    while (dir.size() > 1 && dir[dir.size() - 1] == '/')
        dir.erase(dir.size() - 1);
    return (dir);
}

bool    Post::parse_part(std::string const &part, std::string const &dir)
{
    size_t end_headers = part.find("\r\n\r\n");
    std::string filename;

    if (end_headers == std::string::npos)
        throw ErrorCodes(400);
    filename = get_filename(part.substr(0, end_headers));
    if (filename.empty())
        return (false);
    save_file(dir + "/" + filename, part.substr(end_headers + 4));
    return (true);
}

std::string Post::get_filename(std::string const &part_headers)
{
    size_t start = part_headers.find("filename=\"");
    size_t end;
    std::string filename;

    if (start == std::string::npos)
        return ("");
    start += 10;
    end = part_headers.find('"', start);
    if (end == std::string::npos)
        throw ErrorCodes(400);
    filename = part_headers.substr(start, end - start);
    if (filename.find('/') != std::string::npos || filename.find('\\') != std::string::npos \
        || filename == "." || filename == "..")
        throw ErrorCodes(400);
    return (filename);
}

void    Post::save_file(std::string const &path, std::string const &content)
{
    int fd;
    size_t total;
    ssize_t written;

    fd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1)
        throw ErrorCodes(500);
    total = 0;
    while (total < content.size())
    {
        written = write(fd, content.data() + total, content.size() - total);
        if (written <= 0)
        {
            close(fd);
            throw ErrorCodes(500);
        }
        total += written;
    }
    close(fd);
}
