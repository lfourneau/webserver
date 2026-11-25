#include "Delete.hpp"
#include "ErrorCodes.hpp"
#include <cstdio>
#include <unistd.h>
#include <sys/stat.h>

void    Delete::delete_file(t_execute *exec, t_header const *, t_location const *)
{
    struct stat info;
    std::string parent;
    size_t slash;

    if (stat(exec->filename.c_str(), &info) == -1)
        throw ErrorCodes(404);
    if (S_ISDIR(info.st_mode))
        throw ErrorCodes(409);
    slash = exec->filename.rfind('/');
    if (slash == std::string::npos)
        parent = ".";
    else
        parent = exec->filename.substr(0, slash + 1);
    if (access(parent.c_str(), W_OK | X_OK) == -1)
        throw ErrorCodes(403);
    if (std::remove(exec->filename.c_str()) != 0)
        throw ErrorCodes(500);
    exec->output.clear();
    exec->code = 204;
}
