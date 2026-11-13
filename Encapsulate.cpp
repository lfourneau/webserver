#include "Encapsulate.hpp"
#include "HelperFunctions.hpp"
#include <sstream>
#include <ctime>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

std::string Encapsulate::response(int code, std::string const &body, \
    std::string const &filename, bool keep_alive)
{
    t_headers headers;

    if (code != 204 && code != 304)
        headers["Content-Type"] = content_type(filename);
    headers["Connection"] = keep_alive ? "keep-alive" : "close";
    return (assemble(code, headers, body));
}

std::string Encapsulate::cgi_response(int code, std::string const &cgi_output, \
    bool keep_alive)
{
    t_headers headers;
    std::string body;

    split_cgi_output(cgi_output, headers, body, code);
    if (headers.find("Content-Type") == headers.end())
        headers["Content-Type"] = "text/html; charset=utf-8";
    headers["Connection"] = keep_alive ? "keep-alive" : "close";
    return (assemble(code, headers, body));
}

/*separe les headers ecrits par le script (s'il y en a) du corps de la page*/
void Encapsulate::split_cgi_output(std::string const &cgi_output, t_headers &headers, \
    std::string &body, int &code)
{
    size_t end = cgi_output.find("\r\n\r\n");
    size_t sep = 4;

    body = cgi_output;
    if (end == std::string::npos)
    {
        end = cgi_output.find("\n\n");
        sep = 2;
    }
    if (end == std::string::npos || cgi_output.substr(0, end).find(':') == std::string::npos
        || cgi_output.substr(0, end).find('<') != std::string::npos)
        return ;

    std::istringstream flux(cgi_output.substr(0, end));
    std::string line;

    while (std::getline(flux, line))
    {
        HelperFunctions::trim_right(line, "\r");
        size_t colon = line.find(':');
        if (colon == std::string::npos)
            continue;
        std::string key = line.substr(0, colon);
        std::string value = line.substr(colon + 1);
        HelperFunctions::trim(value, " \t");
        std::string lower = key;
        HelperFunctions::mini(lower);
        if (lower == "status")
            transform_to_num(value, code);
        else if (lower == "content-type")
            headers["Content-Type"] = value;
        else if (lower != "content-length")
            headers[key] = value;
    }
    body = cgi_output.substr(end + sep);
}

/*reponse d'erreur avec le corps deja prepare par HTTP (fichier error_page,
script CGI ou rien). corps vide : page codee en dur*/
std::string Encapsulate::error(int code, std::string const &body, bool keep_alive, \
    std::string const &allow)
{
    t_headers headers;

    headers["Content-Type"] = "text/html; charset=utf-8";
    headers["Connection"] = keep_alive ? "keep-alive" : "close";
    if (code == 405)
        headers["Allow"] = allow;
    return (assemble(code, headers, body.empty() ? default_error_page(code) : body));
}

/*corps d'une page d'erreur generee par CGI : on retire les headers du script.
le code de la reponse reste celui de l'erreur, meme si le script envoie un Status*/
std::string Encapsulate::error_body_from_cgi(std::string const &cgi_output)
{
    t_headers ignored;
    std::string body;
    int ignored_code = 0;

    split_cgi_output(cgi_output, ignored, body, ignored_code);
    return (body);
}

std::string Encapsulate::redirect(int code, std::string const &location)
{
    t_headers headers;
    std::string body;

    body = "<!DOCTYPE html>\n<html><head><title>" + HelperFunctions::ft_itoa(code) + " "
        + ErrorCodes(code).getmessage() + "</title></head>\n<body><p>Moved to <a href=\""
        + location + "\">" + location + "</a></p></body></html>\n";
    headers["Location"] = location;
    headers["Content-Type"] = "text/html; charset=utf-8";
    headers["Connection"] = "keep-alive";
    return (assemble(code, headers, body));
}

std::string Encapsulate::content_type(std::string const &filename)
{
    static char const *ext[] = {".html", ".htm", ".css", ".js", ".txt", ".json",
        ".png", ".jpg", ".jpeg", ".gif", ".svg", ".ico", ".pdf", ".xml", ".mp4",
        ".mp3", ".webp", ".py", ".php", NULL};
    static char const *types[] = {"text/html; charset=utf-8", "text/html; charset=utf-8",
        "text/css", "text/javascript", "text/plain; charset=utf-8", "application/json",
        "image/png", "image/jpeg", "image/jpeg", "image/gif", "image/svg+xml",
        "image/x-icon", "application/pdf", "application/xml", "video/mp4",
        "audio/mpeg", "image/webp", "text/html; charset=utf-8", "text/html; charset=utf-8"};
    size_t dot = filename.rfind('.');
    size_t slash = filename.rfind('/');

    if (dot == std::string::npos || (slash != std::string::npos && dot < slash))
        return ("application/octet-stream");
    std::string extension = filename.substr(dot);
    HelperFunctions::mini(extension);
    for (int i = 0; ext[i] != NULL; i++)
    {
        if (extension == ext[i])
            return (types[i]);
    }
    return ("application/octet-stream");
}

std::string Encapsulate::status_line(int code)
{
    std::ostringstream line;

    line << "HTTP/1.1 " << code << " " << ErrorCodes(code).getmessage() << "\r\n";
    return (line.str());
}

std::string Encapsulate::assemble(int code, t_headers headers, std::string const &body)
{
    std::ostringstream out;
    bool no_body = (code == 204 || code == 304 || (code >= 100 && code < 200));

    headers["Server"] = SERVER_NAME;
    headers["Date"] = current_date();
    if (!no_body)
    {
        std::ostringstream len;
        len << body.size();
        headers["Content-Length"] = len.str();
    }
    else
        headers.erase("Content-Type");
    out << status_line(code);
    for (t_headers::const_iterator it = headers.begin(); it != headers.end(); ++it)
        out << it->first << ": " << it->second << "\r\n";
    out << "\r\n";
    if (!no_body)
        out << body;
    return (out.str());
}

std::string Encapsulate::default_error_page(int code)
{
    std::ostringstream page;
    std::string const message = ErrorCodes(code).getmessage();

    page << "<!DOCTYPE html>\n"
        << "<html>\n<head>\n<meta charset=\"utf-8\">\n"
        << "<title>" << code << " " << message << "</title>\n</head>\n"
        << "<body style=\"font-family: sans-serif; text-align: center; margin-top: 10%;\">\n"
        << "<h1>" << code << "</h1>\n<p>" << message << "</p>\n"
        << "<hr>\n<small>" << SERVER_NAME << "</small>\n"
        << "</body>\n</html>\n";
    return (page.str());
}

bool Encapsulate::read_error_file(std::string const &path, std::string &out)
{
    struct stat info;
    char buffer[4096];
    ssize_t total;
    int fd;

    if (stat(path.c_str(), &info) == -1 || !S_ISREG(info.st_mode))
        return (false);
    fd = open(path.c_str(), O_RDONLY);
    if (fd == -1)
        return (false);
    while ((total = read(fd, buffer, sizeof(buffer))) > 0)
        out.append(buffer, total);
    close(fd);
    return (total == 0);
}

std::string Encapsulate::current_date()
{
    char buffer[64];
    time_t now = time(NULL);
    struct tm *gmt = gmtime(&now);

    strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", gmt);
    return (std::string(buffer));
}
