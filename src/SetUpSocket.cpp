# include "SetUpSocket.hpp"
# include "ServerConfig.hpp"
# include "HTTP.hpp"

SetUpSocket::SetUpSocket(ServerConfig *config) : domain(AF_INET), \
type(SOCK_STREAM), protocol(0), return_getaddr_info(NULL), config(config)
{
    std::memset(&this->parameter, 0, sizeof(this->parameter));
    parameter.ai_family = domain;
    parameter.ai_socktype = type;
    parameter.ai_flags = AI_PASSIVE;

    // une erreur ici (port deja pris, interface inconnue...) remonte jusqu'au
    // main, qui l'affiche et quitte avec un code d'erreur
    for_each_block();
    check_virtual_hosts();
    monitor_connections();
}

SetUpSocket::SetUpSocket(SetUpSocket const &other) : domain(other.domain), \
type(other.type), protocol(other.protocol), port(other.port)
{
    ;
}

SetUpSocket &SetUpSocket::operator=(SetUpSocket const &other)
{
    (void)other;
    return (*this);
}

SetUpSocket::~SetUpSocket() throw()
{
    std::map<int, HTTP *>::iterator client = clients.begin();

    for (; client != clients.end(); ++client)
    {
        delete client->second;
        close(client->first);
    }
    for (std::map<int, int>::iterator it = fds_listen.begin(); it != fds_listen.end(); ++it)
        close(it->first);
    for (size_t i = 0; i < structures_addr_info.size(); i++)
        freeaddrinfo(structures_addr_info[i]);
}

void    SetUpSocket::for_each_block()
{
    std::vector<t_server>::const_iterator it;

    for (it = this->config->get_server_config().begin(); \
    it != this->config->get_server_config().end() ; it++)
    {
        get_addr(*it);
    }
}

//dans cette fonction on complete les informations fournies dans le fichier de configuration
//si il n'y a pas de nom de domaine on rajoute 0.0.0.0 ensuite on verifie que les interfaces
//que nous demandons pointent bien a des paires ip:port different pour s'assurer que
//quand on cherchera a bind il n'y aura pas d'erreur
//on stocke finalement les files descriptors associes aux paries. ip port
void    SetUpSocket::get_addr(t_server const &serv)
{
    std::vector<std::pair<std::string, int> >::const_iterator it;
    std::set<int> used_by_this_server;

    for (it = serv.listen_addrs.begin(); it != serv.listen_addrs.end(); ++it)
    {
        struct addrinfo *new_addrinfo = NULL;
        std::ostringstream to_string;
        std::string name;
        int fd;

        to_string << it->second;
        if (it->first.empty())
        {
            name = "0.0.0.0:" + to_string.str();
            if (bound_addrs.count(name))
                fd = bound_addrs[name];
            else
            {
                fd = wildcard_socket(it->second, name);
                bound_addrs[name] = fd;
            }
        }
        else
        {
            if (getaddrinfo(it->first.c_str(), to_string.str().c_str(), \
                &this->parameter, &new_addrinfo) != 0)
                throw(SocketSetUpFail("listen " + it->first + ":" + to_string.str() \
                    + ": unknown interface"));
            structures_addr_info.push_back(new_addrinfo);
            struct sockaddr_in *addr = reinterpret_cast<struct sockaddr_in *>(new_addrinfo->ai_addr);
            //le nom de l'adresse ip est contenu dans addr->sin_addr il faut le reprendre de la
            unsigned char *bytes = reinterpret_cast<unsigned char *>(&addr->sin_addr);
            std::ostringstream ip_str;
            ip_str << static_cast<int>(bytes[0]) << "." << static_cast<int>(bytes[1]) << "."
                << static_cast<int>(bytes[2]) << "." << static_cast<int>(bytes[3]);
            name = ip_str.str() + ":" + to_string.str();
            if (bound_addrs.count(name))
                fd = bound_addrs[name];
            else
            {
                fd = socket_creation(new_addrinfo->ai_family, new_addrinfo->ai_addr, \
                    new_addrinfo->ai_addrlen, it->second, name);
                bound_addrs[name] = fd;
            }
        }
        if (used_by_this_server.count(fd))
            continue;
        used_by_this_server.insert(fd);
        listen_servers[fd].push_back(serv);
    }
}

int     SetUpSocket::socket_creation(int family, struct sockaddr *addr, socklen_t addr_len, \
    int port, std::string const &name)
{
    int new_listen_fd;

    new_listen_fd = socket(family, SOCK_STREAM, 0);
    if (new_listen_fd == -1)
        throw (SocketSetUpFail("listen " + name + ": socket creation has failed"));
    fds_listen[new_listen_fd] = port;
    set_sock_non_crash(new_listen_fd);
    set_non_blocking(new_listen_fd);
    if (family == AF_INET6)
    {
        int no = 0;

        setsockopt(new_listen_fd, IPPROTO_IPV6, IPV6_V6ONLY, &no, sizeof(no));
    }
    if (bind(new_listen_fd, addr, addr_len))
        throw(SocketSetUpFail("listen " + name + ": bind failed (address already in use?)"));
    if (::listen(new_listen_fd, SOMAXCONN))
        throw(SocketSetUpFail("listen " + name + ": listen failed"));
    return (new_listen_fd);
}

//cette fonction permet de gerer les blocs server dans lesquels un port ne
//serait pas precise. Le port par default est 8080
int     SetUpSocket::wildcard_socket(int port, std::string const &name)
{
    struct sockaddr_in6 addr6;
    struct sockaddr_in addr4;
    int probe;

    probe = socket(AF_INET6, SOCK_STREAM, 0);
    if (probe != -1)
    {
        close(probe);
        std::memset(&addr6, 0, sizeof(addr6));
        addr6.sin6_family = AF_INET6;
        addr6.sin6_addr = in6addr_any;
        addr6.sin6_port = htons(port);
        return (socket_creation(AF_INET6, reinterpret_cast<struct sockaddr *>(&addr6), \
            sizeof(addr6), port, name));
    }
    std::memset(&addr4, 0, sizeof(addr4));
    addr4.sin_family = AF_INET;
    addr4.sin_addr.s_addr = htonl(INADDR_ANY);
    addr4.sin_port = htons(port);
    return (socket_creation(AF_INET, reinterpret_cast<struct sockaddr *>(&addr4), \
        sizeof(addr4), port, name));
}

//notre serveur est en mesure de pouvoir associer plusieurs noms de domaine
//a une seule combinaison adresse ip:port. verifions qu'il n'y ait pas d'incoherences
//par exemple plusieurs blocs servers qui pointernt vers la meme combinaison server/ip
//ou avec des hostnames identiques
void    SetUpSocket::check_virtual_hosts()
{
    std::map<std::string, int>::const_iterator it;

    for (it = bound_addrs.begin(); it != bound_addrs.end(); ++it)
    {
        std::vector<t_server> const &list = listen_servers[it->second];
        std::set<std::string> names;
        bool unnamed = false;

        for (size_t i = 0; i < list.size(); i++)
        {
            if (list[i].server_names.empty())
            {
                if (unnamed)
                    throw(SocketSetUpFail("listen " + it->first \
                        + ": several server blocks without server_name"));
                unnamed = true;
            }
            for (size_t j = 0; j < list[i].server_names.size(); j++)
            {
                if (!names.insert(list[i].server_names[j]).second)
                    throw(SocketSetUpFail("listen " + it->first + ": server_name "
                        + list[i].server_names[j] + " is used twice"));
            }
        }
    }
}

void    SetUpSocket::set_sock_non_crash(int &file_desc)
//on ajoute des flags pour que en case de redemarrage type control c le bind puisse
//se faire (sinon typiquement il faudrait attendre 1min)
{
    int option = true;

    if (setsockopt(file_desc, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) < 0)
        throw(SocketSetUpFail("setsockopt has failed"));
}

//poll s'active uniquement quand un des fd est diponible
//ici on verifie que read accept ou recv ne fasse pas freeze la maichine
void    SetUpSocket::set_non_blocking(int fd)
{
    fcntl(fd, F_SETFL, O_NONBLOCK);
}

//on lance le poll qui permet de monitorer les connections en arrivee
//dispatch va determiner ce qu'il faut faire a savoir accepter une connection
//ecouter la demande ecrire dans le file descriptor et envoyer une reponse
void    SetUpSocket::monitor_connections()
{
    std::vector<struct pollfd> pfds;
    std::vector<t_poll_owner> owners;
    std::set<int> touched;
    int timeout;
    int ready;

    while (!g_stop)
    {
        timeout = build_poll_list(pfds, owners);
        if (pfds.empty())
            return ;
        ready = poll(&pfds[0], pfds.size(), timeout);
        touched.clear();
        for (size_t i = 0; ready > 0 && i < pfds.size(); i++)
        {
            if (pfds[i].revents != 0)
                dispatch(pfds[i], owners[i], touched);
        }
        check_timeouts(touched);
    }
}

//dans poll on ne peut que transmettre une quantite d'informations limitee 
//sur les file descriptors surveilles. ici on cree des structures paralleles
//pour pouvoir savoir comment traiter le fd
int     SetUpSocket::build_poll_list(std::vector<struct pollfd> &pfds, \
    std::vector<t_poll_owner> &owners)
{
    int timeout = POLL_TIMEOUT;

    pfds.clear();
    owners.clear();
    for (std::map<int, int>::const_iterator it = fds_listen.begin(); \
        it != fds_listen.end(); ++it)
        add_poll_entry(pfds, owners, it->first, POLLIN, FD_LISTEN, -1, NULL, 0);
    for (std::map<int, HTTP *>::const_iterator it = clients.begin(); \
        it != clients.end(); ++it)
    {
        HTTP *conn = it->second;

        add_poll_entry(pfds, owners, it->first, conn->client_events(), \
            FD_CLIENT, it->first, conn, 0);
        if (conn->cgi_fd_in() != -1)
            add_poll_entry(pfds, owners, conn->cgi_fd_in(), POLLOUT, \
                FD_CGI_PIPE, it->first, conn, conn->get_cgi_generation());
        if (conn->cgi_fd_out() != -1)
            add_poll_entry(pfds, owners, conn->cgi_fd_out(), POLLIN, \
                FD_CGI_PIPE, it->first, conn, conn->get_cgi_generation());
        if (conn->needs_fast_tick())
            timeout = 10;
    }
    return (timeout);
}

void    SetUpSocket::add_poll_entry(std::vector<struct pollfd> &pfds, \
    std::vector<t_poll_owner> &owners, int fd, short events, \
    e_fd_type type, int client_fd, HTTP *conn, unsigned int generation)
{
    struct pollfd entry;
    t_poll_owner owner;

    entry.fd = fd;
    entry.events = events;
    entry.revents = 0;
    owner.type = type;
    owner.client_fd = client_fd;
    owner.conn = conn;
    owner.generation = generation;
    pfds.push_back(entry);
    owners.push_back(owner);
}

//dans cette fonction on determine exactement ce qu'on doit faire selon le status du 
//du file descriptor on check a la fin le type de requete que l'on peut faire sur 
//file descriptor, lecture ecriture ou erreur, on compare avec revents qui sont les
//infos renvoyees par le noyeau pour signifier des erreures
void    SetUpSocket::dispatch(struct pollfd const &pfd, t_poll_owner const &owner, \
    std::set<int> &touched)
{
    if (owner.type == FD_LISTEN)
    {
        accept_client(pfd.fd, touched);
        return ;
    }
    std::map<int, HTTP *>::iterator it = clients.find(owner.client_fd);
    if (touched.count(owner.client_fd) || it == clients.end() || it->second != owner.conn)
        return ;
    HTTP *conn = owner.conn;
    if (owner.type == FD_CGI_PIPE)
    {
        if (conn->get_cgi_generation() == owner.generation
            && (pfd.fd == conn->cgi_fd_in() || pfd.fd == conn->cgi_fd_out()))
            conn->on_cgi_event(pfd.fd, pfd.revents);
        return ;
    }
    bool keep = true;
    if ((pfd.revents & POLLIN) && (pfd.events & POLLIN))
        keep = conn->on_readable();
    else if ((pfd.revents & POLLOUT) && (pfd.events & POLLOUT))
        keep = conn->on_writable();
    else if (pfd.revents & (POLLHUP | POLLERR | POLLNVAL))
        keep = false;
    if (!keep)
        close_client(pfd.fd, touched);
}

void    SetUpSocket::accept_client(int listen_fd, std::set<int> &touched)
{
    int fd;

    fd = accept(listen_fd, NULL, NULL);
    if (fd == -1)
        return ;
    set_non_blocking(fd);
    try
    {
        clients[fd] = new HTTP(fd, listen_servers[listen_fd]);
    }
    catch (std::exception const &)
    {
        clients.erase(fd);
        close(fd);
        return ;
    }
    touched.insert(fd);
}

//on tue et on libere la memoire du client
void    SetUpSocket::close_client(int fd, std::set<int> &touched)
{
    std::map<int, HTTP *>::iterator it = clients.find(fd);

    if (it == clients.end())
        return ;
    delete it->second;
    clients.erase(it);
    close(fd);
    touched.insert(fd);
}

//on met en place un systeme qui permet d'eviter qu'un programme
//reste bloque sur un waitpid
void    SetUpSocket::check_timeouts(std::set<int> &touched)
{
    std::vector<int> to_close;
    time_t now = time(NULL);

    for (std::map<int, HTTP *>::iterator it = clients.begin(); it != clients.end(); ++it)
    {
        if (!it->second->on_tick(now))
            to_close.push_back(it->first);
    }
    for (size_t i = 0; i < to_close.size(); i++)
        close_client(to_close[i], touched);
}
