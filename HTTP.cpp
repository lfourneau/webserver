#include "HTTP.hpp"
#include "CGI.hpp"
#include "Post.hpp"
#include "ErrorCodes.hpp"
#include "Delete.hpp"
#include "ServerParser.hpp"
#include "Chunked.hpp"
#include "Body.hpp"
#include "FindServer.hpp"

//Les constructeurs

HTTP::HTTP(int fd, std::vector<t_server> const &servers) : fd(fd), servers(servers), header_http(NULL)
{

}

HTTP::HTTP(HTTP const &other) : fd(other.fd), servers(other.servers), \
http_request(other.http_request), header_http(NULL)
{
    if (other.header_http != NULL)
        header_http = new t_header(*other.header_http);
}

HTTP &HTTP::operator=(HTTP const &other)
{
    if (this != &other)
    {
        fd = other.fd;
        servers = other.servers;
        http_request = other.http_request;
        delete header_http;
        header_http = NULL;
        if (other.header_http != NULL)
            header_http = new t_header(*other.header_http);
    }
    return (*this);
}

HTTP::~HTTP()
{
    delete header_http;
}

bool HTTP::build_request()
/*il faut checker que toute la requete a ete recue;
il faut identifier le serveur auquel la requete fait reference
il faut trouver le bloc location qui importe
il faudra gerer les erreures eventuelles*/
{
    if (header_http == NULL && http_request.find("\r\n\r\n") == std::string::npos)
        return (false);
    if (header_http == NULL && http_request.find("\r\n\r\n") != std::string::npos)
        header_http = ServerParser::parse_input(http_request, http_request.find("\r\n\r\n"));
    if (body_complete())
    {
        t_server const &server = FindServer::find_server(header_http, servers);
        t_location const &location = FindServer::find_location(header_http, server);

    }
    return (true);
}

bool HTTP::body_complete()
{
    if (!Chunked::check_chunked(header_http, http_request))
        return (false);
    Body::check_body_size(header_http, http_request);
    return (true);
}

bool HTTP::append_content(const char *cont, unsigned int size)
{
    http_request.append(cont, size);
    return (build_request());
}

int HTTP::get_client_fd() const
{
    return (fd);
}


//HTTP::HTTP(int fd, std::vector<t_server> const &serv) : server_rules(serv), \
//status(PARSE_REQUEST), code(200), error_message("OK"), associated_fd(fd)
//{
//}
//
//HTTP::HTTP(const HTTP &other) : server_rules(other.server_rules)
//{
//    this->associated_fd = other.associated_fd;
//    this->status = other.status;
//    this->request_content = other.request_content;
//    this->content = other.content;
//    this->body = other.body;
//    this->code = other.code;
//    this->info_request = other.info_request;
//    this->error_message = other.error_message;
//    this->loc = other.loc;
//    this->bloc = other.bloc;
//}
//
//HTTP    &HTTP::operator=(const HTTP &other)
//{
//    if (this != &other)
//    {
//        this->associated_fd = other.associated_fd;
//        this->status = other.status;
//        this->request_content = other.request_content;
//        this->content = other.content;
//        this->body = other.body;
//        this->code = other.code;
//        this->info_request = other.info_request;
//        this->error_message = other.error_message;
//        this->loc = other.loc;
//        this->bloc = other.bloc;
//    }
//    return (*this);
//}
//
//HTTP::~HTTP() throw()
//{
//
//}
//
////HelperFunctions not usefull here
//int  HTTP::get_client_fd()
//{
//    return (this->associated_fd);
//}
//
//void HTTP::print_content()
//{
//    std::cout << request_content;
//}
//
//void    HTTP::get_value_host()
//{
//    std::string host_val;
//   
//    try
//    {
//        host_val = info_request.at("Host");
//    }
//    catch(const std::exception& e)
//    {
//        (void)(e);
//        throw(ErrorCodes(400));
//    }
//    std::stringstream info;
//
//    info << host_val;
//    getline(info, info_request["Host_name"], ':');
//    getline(info, info_request["Host_port"]);
//}
//
////
////Fonctionnement de la classe
////recuperer le contenu envoyer par chunk par le serveur
////parser premiere linge
////sauvegarder sous forme de paire le registe
////ouvrir le fichier associer
////l'envoyer
//
//int HTTP::append_content(const char *append_info, unsigned int total_read)
//{
//    request_content.append(append_info, total_read);
//    if (this->request_content.find("\r\n\r\n") != std::string::npos)
//    {
//        try
//        {
//            if (status == PARSE_REQUEST)
//            {
//                analyse_buffer();
//            }
//            if (all_body_received())
//            {
//                perform_request();
//                std::cout << "******************************" << std::endl;
//                std::cout << request_content << std::endl;
//            }
//            else 
//                return (0);
//        }
//        catch(const ErrorCodes &e)
//        {
//            std::string err = e.what();
//            int val = e.getcode();
//            code = e.getcode();
//            error_message = e.what();
//            std::cerr << "Error: " << err << '\n';
//            build_error(val, err);
//        }
//        encapsulate_response();
//        close(associated_fd);
//        return (1);
//    }
//    return (0);
//}
//
//bool    HTTP::all_body_received()
//{
//    if (info_request["Method"] == "GET" || (info_request["Method"] == "POST" && \
//        loc.cgi_pass == true) || info_request["Method"] == "DELETE")
//        return (true);
//
//    if (info_request["Method"] == "POST")
//    {
//        std::map<std::string, std::string>::iterator it;
//
//        it = info_request.find("Content-Length");
//        if (it == info_request.end())
//            throw (ErrorCodes(400));
//        if (static_cast<long unsigned int>(transform_to_num<int>(info_request["Content-Length"])) == body.size())
//            return (true);
//        return (false);
//    }
//    return (true);
//}
//
//void HTTP::analyse_buffer()
//{
//    //dans cette commande il faudrait peut etre 
//    //recuperer le contenu de ma requete puis ensuite proceder a une
//    //encapsulation
//    std::stringstream flux;
//    std::string server_response;
//
//    flux << this->request_content;
//    parse_initial_response_line(flux);
//    parse_header_lines(flux);
//    status = PARSE_BODY;
//    http_body(flux);
//    get_value_host();
//    find_bloc_request();
//    status = APPLY_RULES;
//}
//
//void    HTTP::build_error(int error_code, std::string meaning)
//{
//    HTTP errorinst(*this);
//    //on doit modifier le contenu de la classe HTTP pcq elle peut crash a n'importe quel moment
//    errorinst.info_request["Method"] = "POST";
//    errorinst.info_request["File"] = this->bloc.error_files[404];
//    errorinst.info_request["Protocol"] = "HTTP/1.1";
//    errorinst.info_request["Content-Type"] = "text/html";
//    errorinst.body = TransformToString() << "code=" << error_code << "meaning=" << meaning;
//    errorinst.code = error_code;
//    errorinst.error_message = meaning;
//    errorinst.loc.cgi_pass = true;
//    errorinst.cgi_content();
//    this->content = errorinst.content;
//}
//
//void    HTTP::perform_request()
//{
//    //il faudra creer des strucutes qui autorisent qqn d'externe a lire ecrire ou executer 
//    //un document
//    if (info_request["Method"] == "GET" || (info_request["Method"] == "POST" && loc.cgi_pass == true))
//    {
//        if (!check_authorized_method(info_request["Method"]))
//            throw(ErrorCodes(403));
//        if (loc.cgi_pass == false)
//            read_file();
//        else
//        {
//            code = 201;
//            cgi_content();
//        }
//    }
//    else if (info_request["Method"] == "POST")
//    {
//        Post post_file(*this);
//
//        if (!(check_authorized_method(info_request["Method"])))
//            throw(ErrorCodes(403));
//        post_file.request_post();
//        code = 201;
//    }
//    else if (info_request["Method"] == "DELETE")
//    {
//        Delete delete_file(*this);
//
//        delete_file.request_delete();
//        code = 204;
//    }
//    else 
//        throw(ErrorCodes(405));
//}
//
//bool HTTP::check_authorized_method(std::string const &val)
//{
//    if (val == "GET" && loc.get == true)
//        return (true);
//    else if (val == "POST" && loc.post == true)
//        return (true);
//    else if (val == "DELETE" && loc.del == true)
//        return (true);
//    return (false);
//}
//
//bool HTTP::find_location_block(t_location &find)
//{
//    int longest_match = 0;
//    size_t pos;
//    if (!contains_a_file() && info_request["File"][info_request["File"].size() - 1] != '/')
//    {
//        info_request["File"] += '/';
//        throw(ErrorCodes(301));
//    }
//    for (std::vector<s_location>::iterator it = this->bloc.location_blocks.begin(); \
//    it != this->bloc.location_blocks.end(); \
//    ++it)
//    {
//        if (it->cgi_pass == false)
//        {
//            const long unsigned int size_comparison = info_request["File"].rfind('/');
//            if (size_comparison == std::string::npos)
//                continue;
//            pos = info_request["File"].compare(size_comparison, it->prefix.size(), it->prefix);
//            if (pos == 0)
//            {
//                if (static_cast<int>(it->prefix.size()) > longest_match)
//                {
//                    longest_match = static_cast<int>(it->prefix.size());
//                    find = *it;
//                }
//            }
//        }
//        else
//        {
//            int index_extension;
//            
//            index_extension = info_request["File"].rfind('.');
//            if (index_extension < 0)
//                continue;
//            if (!info_request["File"].compare(index_extension, std::string::npos, it->prefix))
//            {
//                find = *it;
//                longest_match = 1;
//                break;
//            }
//        }
//    }
//    if (longest_match != 0)
//    {
//        if (this->loc.file_directory != "")
//            info_request["File"] = this->loc.file_directory + info_request["File"];
//        else
//            info_request["File"] = this->bloc.server_repo + info_request["File"];
//        if (!contains_a_file())
//            update_file_path();
//        if (!contains_a_file() && loc.autoindex == true)
//            info_request["File"] = info_request["File"] + "autoindex.html";
//        determine_extension();
//        return (true);
//    }
//    throw(ErrorCodes(404));
//}
//
//void HTTP::parse_initial_response_line(std::stringstream &flux)
//{
//    std::string first_line;
//
//    if (!getline(flux, first_line))
//    {
//            throw(ErrorCodes(400));
//    }
//    std::istringstream input(first_line);
//    std::string extra_values;
//
//    // la synthaxe renvoie vrai si on a pu lire et faux dans le cas contraire
//    if (!(input >> info_request["Method"] >> info_request["File"] >> \
//        info_request["Protocol"]) || (input >> extra_values))
//    {
//        throw(ErrorCodes(400));
//    }
//    status = PARSE_HEADER;
//
//    //il faut checker le cgi apres avoir parse les headers
//    check_cgi_request();
//}
//
//void HTTP::parse_header_lines(std::stringstream &flux)
//{
//    while (true)
//    {
//        std::string current_line;
//        std::getline(flux, current_line);
//
//        if (current_line.empty() == true || HelperFunctions::isblank(current_line))
//            break;
//        if (current_line.empty() == false && (current_line[0] == ' ' || current_line[0] == '\t'))
//        {
//            add_to_previous_pair(current_line);
//            continue;
//        }
//
//        std::istringstream line_cut(current_line);
//        std::string key;
//        std::string value;
//
//        //attention cela ne marche que dans le cadre de paires
//        //si c'est une liste il faut separer avec le ":"
//        //ici il faudrait peut etre verifier la presence de ce charactere :
//        getline(line_cut, key, ':');
//        if (line_cut.eof())
//        {
//            std::cout << "Pas de colon identifie" << std::endl;
//            return;
//        }
//        getline(line_cut, value);
//        value = HelperFunctions::trim(value, "\r ");
//        if (key.empty() == true || value.empty() == true)
//            throw(ErrorCodes(400));
//        this->info_request[key] =  value;
//    }
//}
//
//void    HTTP::add_to_previous_pair(std::string &content)
//{
//
//    if (this->info_request.empty() == true)
//    {
//        std::cout << "there is an error with the request";
//        return;
//    }
//
//    std::map<std::string, std::string>::reverse_iterator it = info_request.rbegin();
//    it->second.append(content);
//}
//
//void    HTTP::http_body(std::stringstream &flux)
//{
//    std::ostringstream tmp;
//    tmp << flux.rdbuf();
//    body = tmp.str();
//}
//
//// ici on lit et on stocke le contenu du fichier dans une string
//void    HTTP::read_file()
//{
//    int file_desc;
//
//    if (!contains_a_file() && this->info_request["File"][info_request["File"].length() - 1] != '/')
//    {
//        info_request["File"] += '/';
//        throw(ErrorCodes(301));
//    }
//    else if (!contains_a_file() && this->loc.autoindex == true)
//    {
//        //ici il faut envoyer un fichier qui montre tous les
//        //de l'index
//    }
//    else if (!contains_a_file() && this->loc.autoindex == false)
//    {
//        throw (ErrorCodes(403));
//    }
//    file_desc = open(info_request["File"].c_str(), O_RDONLY);
//    if (file_desc == -1)
//        throw(ErrorCodes(404));
//    read_and_store_file(file_desc);
//}
//
//void HTTP::update_file_path()
//{
//    if (this->loc.default_file != "")
//    {
//        info_request["File"] = info_request["File"] + loc.default_file;
//    }
//}
//
//bool HTTP::contains_a_file()
//{
//    size_t rightmost_dash;
//    size_t rightmost_dot;
//
//    rightmost_dash = info_request["File"].rfind("/");
//    rightmost_dot = info_request["File"].rfind(".");
//
//    if (rightmost_dot != std::string::npos && (rightmost_dot - rightmost_dash > 1 || \
//    rightmost_dash != std::string::npos) )
//        return (true);
//    return (false);
//}
//
//bool    HTTP::check_cgi_request()
//{
//    if (info_request["File"].find("?") == std::string::npos)
//        return(false);
//
//    std::stringstream line;
//    line << info_request["File"];
//    info_request["CGI params"] = "";
//    info_request["File"].clear();
//    getline(line, info_request["File"],  '?');
//    getline(line, info_request["CGI params"]);
//    return (true);
//}
//
//void    HTTP::determine_extension()
//{
//    size_t last_slash;
//    size_t exstension;
//    std::string file_exstensions[] = {".html", ".htm", ".css", ".js", ".txt", ".json", ".png", ".jpg", \
//        ".jpeg", ".gif", ".svg", ".ico", ".pdf", ".py"};
//    std::string file_type[] = {"text/html", "text/html", "text/css", "text/javascript", "text/plain", \
//        "application/json", "image/png", "image/jpeg", "image/jpeg", "image/gif", "image/svg+xml", \
//        "image/x-ico", "application/pdf", "text/html" };
//
//    last_slash = info_request["File"].rfind('/');
//    exstension = info_request["File"].rfind('.');
//    if (exstension == std::string::npos && last_slash != std::string::npos)
//    {
//        info_request["Content-Type"] = std::string("application/octet-stream");
//        return ;
//    }
//    for (int i = 0; static_cast<unsigned long>(i) < sizeof(file_exstensions) / sizeof(file_exstensions[0]); i++)
//    {
//        if (info_request["File"].find(file_exstensions[i]) != std::string::npos)
//        {
//            info_request["Content-Type"] = file_type[i];
//            return ;
//        }
//    }
//    //attention ne fonctionne pas si c'est un fichier .php
//    info_request["Content-Type"] = std::string("text/html");
//}
//
//void    HTTP::read_and_store_file(int fd)
//{
//    char    buffer[4096];
//    int  current_buffer;
//
//    bzero(buffer, 4096);
//    while (true)
//    {
//        current_buffer = read(fd, buffer, 4096);
//        if (current_buffer == 0)
//        {
//            close (fd);
//            break;
//        }
//        content.append(buffer, current_buffer);
//        bzero(buffer, 4096);
//    }
//}
//
////on utilise le contenu de cette string pour constuire une reponse 
//
//void    HTTP::cgi_content(void)
//{
//    std::ostringstream output_message;
//    std::string final_content;
//
//    if (loc.cgi_pass == true)
//    {
//        CGI     send_to_cgi(*this);
//
//        content = send_to_cgi.get_output();
//    }
//    return;
//}
//
//void    HTTP::encapsulate_response()
//{
//    std::ostringstream output_message;
//    std::string final_message;
//
//    output_message << "HTTP/1.1 " << code << " " << this->error_message << "\r\n" \
//    << "Content-Type: " << info_request["Content-Type"] << "\r\n" << "Content-Length: " << content.size() \
//    << "\r\n" << "Location: " << info_request["File"] << "\r\n\r\n" \
//    << content;
//    final_message = output_message.str();
//    std::cout << "--------------------------------" << std::endl;
//    std::cout << final_message << std::endl;
//    send(this->associated_fd, final_message.c_str(), final_message.size(), 0);
//    return ;
//}
//
//void    HTTP::find_bloc_request()
//{
//    std::vector<t_server>::const_iterator it = this->server_rules.begin();
//    bool bloc_found = false;
//
//    for (; it != this->server_rules.end(); ++it)
//    {
//        int port_to_compare = -1;
//
//        for (std::set<int>::iterator val = it->open_ports.begin(); val != it->open_ports.end(); ++val )
//        {
//            int port_num;
//            transform_to_num(info_request["Host_port"], port_num);
//            if (*val == port_num)
//            {
//                port_to_compare = port_num;
//                break;
//            }
//        }
//        if (port_to_compare == -1)
//            continue;
//        if (bloc_found == false)
//        {
//            bloc = *it;
//            bloc_found = true;
//            continue;
//        }
//        for (std::vector<std::string>::const_iterator server_names = it->server_names.begin(); \
//            server_names != it->server_names.end(); ++server_names)
//        {
//            if (*server_names == info_request["Host_name"])
//            {
//                bloc = *it;
//                bloc_found = true;
//                return;
//            }
//        }
//    }
//    find_location_block(this->loc);
//    //if (!contains_a_file())
//    //{ 
//    //    throw(ErrorCodes(400));
//    //}
//    if (bloc_found == false)
//        throw (ErrorCodes(500));
//}