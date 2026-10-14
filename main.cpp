#include "HTTP.hpp"
#include "SetUpSocket.hpp"

/*
Steps in the creation of a webserver
1) Create a socker
2) Bind it to an IP port
3) Mark the socket for listening in
4) Accpet a call
5) Close the listening socket
6) While receiving the message echo its content
7) Close  the socket*/


int main(void)
{
    SetUpSocket     init(AF_INET, SOCK_STREAM, 0, "5400");

    while (1)
    {
        ;
    }
    return (0);
}

/*
int main(void)
{
    std::string file = "POST /users HTTP/1.1\r\nHost: example.com\r\n\
                        Content-Type: application/x-www-form-urlencoded\r\n\
                        Content-Length: 49\r\n\
                        name=FirstName+LastName&email=bsmth%40example.com";

    HTTP    content(1);
    content.append_content(file);
    content.parse_input(); 
}
*/

