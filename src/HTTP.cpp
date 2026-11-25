#include "HTTP.hpp"
#include "CGI.hpp"
#include "Get.hpp"
#include "ErrorCodes.hpp"
#include "ServerParser.hpp"
#include "Chunked.hpp"
#include "Body.hpp"
#include "FindServer.hpp"
#include "Encapsulate.hpp"
#include <poll.h>
#include <sys/socket.h>

HTTP::HTTP(int fd, std::vector<t_server> const &servers) : fd(fd), servers(&servers), \
http_request(), header_http(NULL), response(), response_sent(0), state(READING_REQUEST), \
last_activity(time(NULL)), server(NULL), loc(NULL), cgi(NULL), cgi_purpose(CGI_REQUEST), \
cgi_generation(0), error_code(0), error_allow(), autoindex_dir(), autoindex_uri()
{
}

HTTP::~HTTP()
{
    delete header_http;
    delete cgi;
}

short HTTP::client_events() const
{
    if (state == SENDING_RESPONSE)
        return (POLLOUT);
    return (POLLIN);
}

int HTTP::cgi_fd_in() const
{
    if (state != RUNNING_CGI || cgi == NULL)
        return (-1);
    return (cgi->get_fd_in());
}

int HTTP::cgi_fd_out() const
{
    if (state != RUNNING_CGI || cgi == NULL)
        return (-1);
    return (cgi->get_fd_out());
}

unsigned int HTTP::get_cgi_generation() const
{
    return (cgi_generation);
}

bool HTTP::needs_fast_tick() const
{
    return (state == RUNNING_CGI && cgi != NULL && cgi->waiting_for_exit());
}

//cette fonction lit le fichier et change le state associe a la requete pour
//que setupsocket sache ou est la connection
//recv envoie 0 si le client est parti
bool HTTP::on_readable()
{
    char buffer[4096];
    ssize_t total;

    total = recv(fd, buffer, sizeof(buffer), 0);
    if (total <= 0)
        return (false);
    if (state != READING_REQUEST)
        return (true);
    last_activity = time(NULL);
    http_request.append(buffer, total);
    handle_request();
    return (true);
}

//on envoie la reponse et on dit si elle a reussie ou echouee
bool HTTP::on_writable()
{
    ssize_t sent;

    sent = send(fd, response.c_str() + response_sent, \
        response.size() - response_sent, MSG_NOSIGNAL);
    if (sent <= 0)
        return (false);
    last_activity = time(NULL);
    response_sent += sent;
    return (response_sent < response.size());
}

//on verifie un chagement d'etat dans revents si une erreure se produit
//elle n'est pas traitee dans 
void HTTP::on_cgi_event(int pipe_fd, short revents)
{
    if (cgi == NULL || state != RUNNING_CGI)
        return ;
    if (pipe_fd == cgi->get_fd_in() && (revents & (POLLOUT | POLLERR | POLLHUP)))
        cgi->on_writable();
    else if (pipe_fd == cgi->get_fd_out() && (revents & (POLLIN | POLLERR | POLLHUP)))
        cgi->on_readable();
    check_cgi(time(NULL));
}

//on verifie que la requete n'a pas pris trop de temps
bool HTTP::on_tick(time_t now)
{
    if (state == RUNNING_CGI)
    {
        check_cgi(now);
        return (true);
    }
    if (now - last_activity < CLIENT_TIMEOUT)
        return (true);
    if (state == READING_REQUEST && !http_request.empty())
    {
        start_error(ErrorCodes(408));
        return (true);
    }
    return (false);
}

void HTTP::handle_request()
{
    try
    {
        build_request();
    }
    catch (ErrorCodes const &err)
    {
        start_error(err);
    }
    catch (std::exception const &)
    {
        start_error(ErrorCodes(500));
    }
}

//construction de la response a renvoyer au client
//verifications de la taille totale du corps de la demande
bool HTTP::build_request()
{
    if (header_http == NULL && http_request.find("\r\n\r\n") == std::string::npos)
    {
        if (http_request.size() > MAX_HEADER_SIZE)
            throw ErrorCodes(431);
        return (false);
    }
    if (header_http == NULL)
    {
        header_http = ServerParser::parse_input(http_request, http_request.find("\r\n\r\n"));
        server = &FindServer::find_server(header_http, *servers);
        if (server->max_body_size > 0 && \
            Body::get_content_length(header_http) > static_cast<size_t>(server->max_body_size))
            throw ErrorCodes(413);
    }
    if (!body_complete())
        return (false);
    if (server->max_body_size > 0 && \
        header_http->body_request.size() > static_cast<size_t>(server->max_body_size))
        throw ErrorCodes(413);
    loc = &FindServer::find_location(header_http, *server);

    ExecuteRequest req(*server, *loc, *header_http);
    req.execute();
    t_execute const &exec = req.get_exec();
    if (!exec.cgi)
    {
        set_response(Encapsulate::response(exec.code, exec.output, exec.filename, false));
        return (true);
    }
    if (exec.autoindex)
    {
        autoindex_dir = exec.filename;
        autoindex_uri = header_http->URI;
        if (!start_cgi(exec.cgi_prog, exec.cgi_script, exec.cgi_env, "", CGI_AUTOINDEX))
            autoindex_fallback();
        return (true);
    }
    if (!start_cgi(exec.cgi_prog, exec.cgi_script, exec.cgi_env, \
        header_http->body_request, CGI_REQUEST))
        throw ErrorCodes(500);
    return (true);
}

bool HTTP::body_complete()
{
    size_t start = http_request.find("\r\n\r\n") + 4;
    size_t len;

    if (header_http->headers.count("transfer-encoding"))
    {
        if (!Chunked::check_chunked(header_http, http_request))
            return (false);
        header_http->body_request = Chunked::unchunk(http_request, start);
    }
    else if (header_http->headers.count("content-length"))
    {
        len = Body::get_content_length(header_http);
        if (http_request.size() - start < len)
            return (false);
        header_http->body_request = http_request.substr(start, len);
    }
    else
        header_http->body_request.clear();
    return (true);
}

void HTTP::start_error(ErrorCodes const &err)
{
    int code = err.getcode();
    std::string script;

    if (code >= 300 && code < 400 && !err.get_uri().empty())
    {
        set_response(Encapsulate::redirect(code, err.get_uri()));
        return ;
    }
    if (server == NULL && !servers->empty())
        server = &(*servers)[0];
    error_code = code;
    error_allow = allowed_methods();
    if (server != NULL)
    {
        std::map<int, std::string>::const_iterator it = server->error_files.find(code);

        if (it != server->error_files.end())
        {
            std::string body;

            if (CGI::interpreter_for(it->second).empty())
            {
                if (!Encapsulate::read_error_file(it->second, body))
                    body.clear();
                set_response(Encapsulate::error(code, body, false, error_allow));
                return ;
            }
            script = it->second;
        }
        else
            script = server->server_repo + "/" + ERROR_SCRIPT;
    }
    if (!script.empty())
    {
        struct stat info;
        std::vector<std::string> env;

        env.push_back("STATUS_CODE=" + HelperFunctions::ft_itoa(code));
        env.push_back("REDIRECT_STATUS=" + HelperFunctions::ft_itoa(code));
        env.push_back("REQUEST_METHOD=GET");
        env.push_back("GATEWAY_INTERFACE=CGI/1.1");
        env.push_back("SCRIPT_FILENAME=" + script);
        if (header_http != NULL)
            env.push_back("REQUEST_URI=" + header_http->URI);
        if (stat(script.c_str(), &info) == 0 && S_ISREG(info.st_mode)
            && start_cgi(CGI::interpreter_for(script), script, env, "", CGI_ERROR_PAGE))
            return ;
    }
    set_response(Encapsulate::error(code, "", false, error_allow));
}

std::string HTTP::allowed_methods() const
{
    std::string allow;

    if (loc == NULL)
        return (allow);
    if (loc->get)
        allow += "GET";
    if (loc->post)
        allow += allow.empty() ? "POST" : ", POST";
    if (loc->del)
        allow += allow.empty() ? "DELETE" : ", DELETE";
    return (allow);
}

bool HTTP::start_cgi(std::string const &prog, std::string const &script, \
    std::vector<std::string> const &env, std::string const &body, e_cgi_purpose purpose)
{
    delete cgi;
    cgi = new CGI();
    if (!cgi->start(prog, script, env, body, purpose == CGI_REQUEST))
    {
        delete cgi;
        cgi = NULL;
        return (false);
    }
    cgi_purpose = purpose;
    cgi_generation++;
    state = RUNNING_CGI;
    return (true);
}

void HTTP::check_cgi(time_t now)
{
    CGI::e_result result;

    if (cgi == NULL || state != RUNNING_CGI)
        return ;
    result = cgi->update(now);
    if (result != CGI::RUNNING)
        finish_cgi(result);
}

/*le script est termine (ou tue) : on construit la reponse*/
void HTTP::finish_cgi(int result)
{
    std::string output = cgi->get_output();

    if (cgi_purpose == CGI_REQUEST)
    {
        if (result == CGI::SUCCESS)
            set_response(Encapsulate::cgi_response(200, output, false));
        else if (result == CGI::TIMEOUT)
            start_error(ErrorCodes(504));
        else
            start_error(ErrorCodes(502));
    }
    else if (cgi_purpose == CGI_AUTOINDEX)
    {
        if (result == CGI::SUCCESS)
            set_response(Encapsulate::cgi_response(200, output, false));
        else
            autoindex_fallback();
    }
    else
    {
        std::string body;

        if (result == CGI::SUCCESS)
            body = Encapsulate::error_body_from_cgi(output);
        set_response(Encapsulate::error(error_code, body, false, error_allow));
    }
}

void HTTP::autoindex_fallback()
{
    try
    {
        set_response(Encapsulate::response(200, \
            Get::autoindex_page(autoindex_dir, autoindex_uri), "index.html", false));
    }
    catch (ErrorCodes const &err)
    {
        start_error(err);
    }
}

void HTTP::set_response(std::string const &content)
{
    response = content;
    response_sent = 0;
    state = SENDING_RESPONSE;
    last_activity = time(NULL);
}

int HTTP::get_client_fd() const
{
    return (fd);
}