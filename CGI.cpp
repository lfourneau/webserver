#include "CGI.hpp"
#include "HelperFunctions.hpp"
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <algorithm>

CGI::CGI() : pid(-1), fd_in(-1), fd_out(-1), body(), body_sent(0), output(), \
start_time(0), result(RUNNING)
{
}

/*le client s'est deconnecte ou la reponse est partie : on ne laisse
ni fd ouvert, ni processus zombie*/
CGI::~CGI()
{
    close_fd(fd_in);
    close_fd(fd_out);
    kill_child();
}

/*lance le script. renvoie false si pipe() ou fork() echoue.
seules les extremites gardees par le serveur passent en O_NONBLOCK :
le script, lui, lit et ecrit normalement.
in_script_dir : le script tourne dans son dossier (exige par le sujet pour
les CGI du client). le chemin du script devient alors son simple nom, sinon
un chemin relatif (root ./) ne serait plus valable apres le chdir.
les scripts internes (erreurs, autoindex) restent dans le dossier du serveur
pour que les chemins relatifs qu'on leur passe restent justes*/
bool CGI::start(std::string const &prog, std::string const &script, \
    std::vector<std::string> const &request_env_vars, std::string const &request_body, \
    bool in_script_dir)
{
    int to_cgi[2];
    int from_cgi[2];
    std::vector<std::string> argv_str;
    std::vector<std::string> env = request_env_vars;
    std::string dir;
    std::string script_arg = script;

    if (in_script_dir && script.rfind('/') != std::string::npos)
    {
        dir = script.substr(0, script.rfind('/') + 1);
        script_arg = script.substr(script.rfind('/') + 1);
        for (size_t i = 0; i < env.size(); i++)
        {
            if (env[i].compare(0, 16, "SCRIPT_FILENAME=") == 0)
                env[i] = "SCRIPT_FILENAME=" + script_arg;
        }
    }

    if (pipe(to_cgi) == -1)
        return (false);
    if (pipe(from_cgi) == -1)
    {
        close(to_cgi[0]);
        close(to_cgi[1]);
        return (false);
    }
    argv_str.push_back(prog);
    argv_str.push_back(script_arg);
    char **argv = to_c_array(argv_str);
    char **envp = to_c_array(env);

    pid = fork();
    if (pid == 0)
    {
        dup2(to_cgi[0], STDIN_FILENO);
        dup2(from_cgi[1], STDOUT_FILENO);
        close(to_cgi[0]);
        close(to_cgi[1]);
        close(from_cgi[0]);
        close(from_cgi[1]);
        // le script ne doit pas heriter des reglages de signaux du serveur
        signal(SIGPIPE, SIG_DFL);
        signal(SIGINT, SIG_DFL);
        signal(SIGTERM, SIG_DFL);
        // on se place dans le dossier du script (chemins relatifs)
        if (!dir.empty() && chdir(dir.c_str()) == -1)
            _exit(1);
        execve(prog.c_str(), argv, envp);
        // execve a echoue : le fils ne doit surtout pas continuer le serveur
        _exit(127);
    }
    free_c_array(argv);
    free_c_array(envp);
    close(to_cgi[0]);
    close(from_cgi[1]);
    fd_in = to_cgi[1];
    fd_out = from_cgi[0];
    if (pid == -1)
    {
        close_fd(fd_in);
        close_fd(fd_out);
        return (false);
    }
    fcntl(fd_in, F_SETFL, O_NONBLOCK);
    fcntl(fd_out, F_SETFL, O_NONBLOCK);
    body = request_body;
    body_sent = 0;
    start_time = time(NULL);
    result = RUNNING;
    // pas de body : on ferme tout de suite, le script lit EOF sur stdin
    if (body.empty())
        close_fd(fd_in);
    return (true);
}

/*poll() a signale POLLOUT sur fd_in : un seul write, puis on rend la main*/
void CGI::on_writable()
{
    ssize_t written;

    if (fd_in == -1)
        return ;
    written = write(fd_in, body.c_str() + body_sent, body.size() - body_sent);
    // -1 : le script a ferme son stdin (ou est mort), inutile d'insister
    if (written <= 0)
    {
        close_fd(fd_in);
        return ;
    }
    body_sent += written;
    if (body_sent >= body.size())
        close_fd(fd_in);
}

/*poll() a signale POLLIN ou POLLHUP sur fd_out : un seul read.
0 = EOF, le script a fini d'ecrire (ou a ferme sa sortie)*/
void CGI::on_readable()
{
    char buffer[4096];
    ssize_t total;

    if (fd_out == -1)
        return ;
    total = read(fd_out, buffer, sizeof(buffer));
    if (total > 0)
    {
        output.append(buffer, total);
        return ;
    }
    close_fd(fd_out);
}

/*appele a chaque tour de boucle : timeout et fin du processus.
waitpid(WNOHANG) ne bloque jamais : il renvoie 0 si le fils tourne encore*/
CGI::e_result CGI::update(time_t now)
{
    int status;
    pid_t ret;

    if (result != RUNNING)
        return (result);
    if (now - start_time >= CGI_TIMEOUT)
    {
        close_fd(fd_in);
        close_fd(fd_out);
        kill_child();
        result = TIMEOUT;
        return (result);
    }
    // tant que la sortie n'est pas finie, on continue a lire
    if (fd_out != -1)
        return (RUNNING);
    ret = waitpid(pid, &status, WNOHANG);
    if (ret == 0)
        return (RUNNING);
    pid = -1;
    close_fd(fd_in);
    if (ret == -1 || !WIFEXITED(status) || WEXITSTATUS(status) != 0)
        result = FAILURE;
    else
        result = SUCCESS;
    return (result);
}

int CGI::get_fd_in() const
{
    return (fd_in);
}

int CGI::get_fd_out() const
{
    return (fd_out);
}

/*sortie terminee mais processus pas encore recupere : la boucle doit
revenir vite (petit timeout de poll) pour appeler update()*/
bool CGI::waiting_for_exit() const
{
    return (result == RUNNING && fd_out == -1);
}

std::string const &CGI::get_output() const
{
    return (output);
}

std::string CGI::interpreter_for(std::string const &script)
{
    size_t dot = script.rfind('.');

    if (dot == std::string::npos)
        return ("");
    if (script.substr(dot) == ".py")
        return (PYTHON_INTERPRETER);
    if (script.substr(dot) == ".php")
        return (PHP_INTERPRETER);
    return ("");
}

/*variables d'environnement d'une requete CGI du client (RFC 3875)*/
std::vector<std::string> CGI::request_env(t_execute const *exec, t_header const *head)
{
    std::vector<std::string> env;
    std::map<std::string, std::string>::const_iterator it = head->headers.begin();

    for (; it != head->headers.end(); ++it)
    {
        std::string name = it->first;

        std::replace(name.begin(), name.end(), '-', '_');
        HelperFunctions::maxi(name);
        if (it->first != "content-type" && it->first != "content-length")
            name.insert(0, "HTTP_");
        env.push_back(name + "=" + it->second);
    }
    // pas de "\r\n" dans l'environnement : la valeur serait polluee
    env.push_back("REQUEST_METHOD=" + head->method);
    env.push_back("SCRIPT_NAME=" + head->URI);
    env.push_back("SCRIPT_FILENAME=" + exec->filename);
    env.push_back("PATH_INFO=" + head->URI);
    env.push_back("REQUEST_URI=" + head->URI);
    env.push_back("QUERY_STRING=" + head->cgi_extension);
    env.push_back("CONTENT_LENGTH=" + HelperFunctions::ft_itoa(head->body_request.size()));
    env.push_back("SERVER_PROTOCOL=" + head->protocol);
    env.push_back("SERVER_NAME=" + head->host_name);
    env.push_back("SERVER_PORT=" + HelperFunctions::ft_itoa(head->host_port));
    env.push_back("SERVER_SOFTWARE=webserv/1.0");
    env.push_back("GATEWAY_INTERFACE=CGI/1.1");
    env.push_back("REMOTE_ADDR=127.0.0.1");
    env.push_back("REDIRECT_STATUS=200");
    return (env);
}

void CGI::close_fd(int &fd)
{
    if (fd != -1)
        close(fd);
    fd = -1;
}

/*SIGKILL puis waitpid : le fils meurt immediatement, l'attente est
instantanee et evite un processus zombie*/
void CGI::kill_child()
{
    int status;

    if (pid <= 0)
        return ;
    kill(pid, SIGKILL);
    waitpid(pid, &status, 0);
    pid = -1;
}

char **CGI::to_c_array(std::vector<std::string> const &vec)
{
    char **array = new char *[vec.size() + 1];

    for (size_t i = 0; i < vec.size(); i++)
    {
        array[i] = new char[vec[i].size() + 1];
        std::strcpy(array[i], vec[i].c_str());
    }
    array[vec.size()] = NULL;
    return (array);
}

void CGI::free_c_array(char **array)
{
    if (array == NULL)
        return ;
    for (size_t i = 0; array[i] != NULL; i++)
        delete[] array[i];
    delete[] array;
}
