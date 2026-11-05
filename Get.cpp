#include "Get.hpp"
#include "ErrorCodes.hpp"

std::string Get::extract_output(t_execute *exec, t_header const *head, t_location const *loc)
{
    if (loc->cgi_pass == true)
        throw(ErrorCodes(500));
    //lire le fichier dans un buffer
    //output le resultat
}

void    Get::read_file(t_execute *exec, std::string file_to_read);
{
    int fd;

    fd = open(exec.c_str(), O_RDONLY, 0644);
    if (fd == -1)
        throw(ErrorCodes(500));
    
}


