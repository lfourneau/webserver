#include "ServerConfig.hpp"

ServerConfig::ServerConfig(std::string &path_config)
{
    std::string content_config;

    open_read_config(content_config, path_config);
//  parse_content(content_config);
    tokenize(content_config);
    recup_information();
    std::cout << "valid file for now" << std::endl;
}

ServerConfig::ServerConfig(ServerConfig const &other)
{
    (void)other;
}

ServerConfig &ServerConfig::operator=(ServerConfig const &other)
{
    (void)other;
    return (*this);
}

ServerConfig::~ServerConfig()
{
}

void ServerConfig::open_read_config(std::string &content, std::string &config)
{
    int fd;
    char buffer[4096];
    ssize_t total_read;

    fd = open(config.c_str(), O_RDONLY);
    if (fd == -1)
    {
        throw (FileOpenFail());
    }
    while (true)
    {
        total_read = read(fd, buffer, 4096);
        if (total_read < 0)
            throw(ReadFail());
        if (total_read == 0)
            break;
        content.append(buffer);
    }
    close(fd);
}

void ServerConfig::tokenize(std::string const &flux)
{
    std::string current_token;

    for (long unsigned int i = 0; i < flux.size(); i++)
    {
        unsigned char current_char = static_cast<unsigned char>(flux[i]);

        if (std::isspace(current_char) && current_token.empty() == false)
        {
            servconfig.tokens.push_back(current_token);
            reinitialize_string(current_token);
        }
        else if (current_char == '#')
        {
            while (current_char != '\n')
                i++;
        }
        else if (current_char == '{' || current_char == '}' || current_char == ';') 
        {
            if (current_token.empty() == false)
            {
                servconfig.tokens.push_back(current_token);
                reinitialize_string(current_token);
            }
            servconfig.tokens.push_back(std::string(1, current_char));
        }
        else if (std::isspace(current_char) == false) 
        {
            current_token += current_char;
        }
    }
    if (current_token.empty() == false)
        servconfig.tokens.push_back(current_token);
;}

void ServerConfig::reinitialize_string(std::string &content)
{
    content = "";
    content.clear();
}

void ServerConfig::recup_information()
{
    std::vector<std::string>::iterator it;
    std::vector<std::string>::iterator end = servconfig.tokens.end();

    for (it = servconfig.tokens.begin(); it != end; ++it)
    {
        if (*it == "server")
        {
            it++;

            Server  new_server_block(it, servconfig.tokens.end());

            servconfig.servers.push_back(new_server_block);
        }
        if (it == end)
            break;
    }
}

t_serverconfig const ServerConfig::get_server_config() const
{
    return (this->servconfig);
}