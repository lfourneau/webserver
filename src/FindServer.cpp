#include "FindServer.hpp"
#include "ErrorCodes.hpp"
#include "Server.hpp"
#include "HTTP.hpp"

t_server const &FindServer::find_server(t_header const *val, std::vector<t_server> const &servers)
{
    if (servers.empty())
        throw(ErrorCodes(500));
    std::vector<t_server>::const_iterator it = servers.begin();
    for (; it != servers.end(); ++it)
    {
        std::vector<std::string>::const_iterator it_nam = it->server_names.begin();
        for (; it_nam != it->server_names.end(); ++it_nam)
        {
            if (val->host_name != "" && val->host_name == *it_nam)
                return (*it);
        }
    }
    return (servers[0]);
}

t_location const &FindServer::find_location(t_header const *val, t_server const &servers)
{
    t_location const *best = NULL;
    unsigned int size_match = 0;
    bool missing_slash = false;
    std::vector<t_location>::const_iterator it = servers.location_blocks.begin();

    for (; it != servers.location_blocks.end(); ++it)
    {
        t_location const *found = NULL;
        if (it->cgi_pass == true)
        {
            found = find_cgi(val, &(*it));
            if (found != NULL)
                return(*found);
        }
        else
        {
            found = find_repo(val, &(*it), size_match);
            if (found != NULL)
                best = found;
            else if (it->prefix.size() > 1 && it->prefix == val->URI + "/")
                missing_slash = true;
        }
    }
    if (missing_slash && size_match < val->URI.size() + 1)
        throw (ErrorCodes(val->method == "GET" ? 301 : 308, val->URI + "/"));
    if (best == NULL)
        throw (ErrorCodes(404));
    return (*best);
}

t_location const *FindServer::find_cgi(t_header const *val, t_location const *loc)
{
    if (val->URI.size() < loc->prefix.size())
        return (NULL);
    if (!val->URI.compare(val->URI.size() - loc->prefix.size(), loc->prefix.size(), loc->prefix))
        return (loc);
    return (NULL);
}

t_location const *FindServer::find_repo(t_header const *val, t_location const *loc, unsigned int &len)
{
    if (val->URI.size() < loc->prefix.size() || loc->prefix.size() < len)
        return (NULL);
    if (!val->URI.compare(0, loc->prefix.size(), loc->prefix))
    {
        len = loc->prefix.size();
        return (loc);
    }
    return (NULL);
}