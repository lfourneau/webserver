#include "Get.hpp"
#include "ErrorCodes.hpp"
#include <dirent.h>
#include <set>

void Get::extract_output(t_execute *exec, t_header const *, t_location const *loc)
{
    if (loc->cgi_pass == true)
        throw(ErrorCodes(500));
    read_file(exec);
}

void    Get::read_file(t_execute *exec)
{
    int fd;
    char buffer[4096];

    fd = open(exec->filename.c_str(), O_RDONLY, 0644);
    if (fd == -1)
        throw(ErrorCodes(500));
    while (true)
    {
        ssize_t total_read;

        total_read = read(fd, buffer, 4096);
        if (total_read == -1)
        {
            close(fd);
            throw(ErrorCodes(500));
        }
        else if (total_read == 0)
            break;
        else
            exec->output.append(buffer, total_read);
    }
    close(fd);
    return; 
}



static std::string html_escape(std::string const &text)
{
    std::string out;

    for (size_t i = 0; i < text.size(); i++)
    {
        if (text[i] == '&')
            out += "&amp;";
        else if (text[i] == '<')
            out += "&lt;";
        else if (text[i] == '>')
            out += "&gt;";
        else if (text[i] == '"')
            out += "&quot;";
        else
            out += text[i];
    }
    return (out);
}

/*listing code en dur, utilise seulement si cgi-bin/autoindex.py plante.
les noms sont tries (std::set) : dossiers d'abord, puis fichiers*/
std::string Get::autoindex_page(std::string const &dir, std::string const &uri)
{
    DIR *directory = opendir(dir.c_str());
    struct dirent *entry;
    std::set<std::string> folders;
    std::set<std::string> files;

    if (directory == NULL)
        throw ErrorCodes(403);
    while ((entry = readdir(directory)) != NULL)
    {
        std::string name = entry->d_name;
        struct stat info;

        if (name == "." || name == "..")
            continue;
        if (stat((dir + name).c_str(), &info) == 0 && S_ISDIR(info.st_mode))
            folders.insert(name + "/");
        else
            files.insert(name);
    }
    closedir(directory);

    std::string page = "<!DOCTYPE html>\n<html>\n<head>\n<meta charset=\"utf-8\">\n"
        "<title>Index of " + html_escape(uri) + "</title>\n</head>\n<body>\n"
        "<h1>Index of " + html_escape(uri) + "</h1>\n<hr>\n<pre>\n";
    if (uri != "/")
        page += "<a href=\"../\">../</a>\n";
    std::set<std::string>::const_iterator it;
    for (it = folders.begin(); it != folders.end(); ++it)
        page += "<a href=\"" + html_escape(*it) + "\">" + html_escape(*it) + "</a>\n";
    for (it = files.begin(); it != files.end(); ++it)
        page += "<a href=\"" + html_escape(*it) + "\">" + html_escape(*it) + "</a>\n";
    page += "</pre>\n<hr>\n</body>\n</html>\n";
    return (page);
}
