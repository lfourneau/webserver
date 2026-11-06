#include "Get.hpp"
#include "ErrorCodes.hpp"

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


