#include "HTTP.hpp"
#include "SetUpSocket.hpp"
#include "ErrorCodes.hpp"
#include "Server.hpp"

/*
Steps in the creation of a webserver
1) Create a socker
2) Bind it to an IP port
3) Mark the socket for listening in
4) Accpet a call
5) Close the listening socket
6) While receiving the message echo its content
7) Close  the socket*/

//int main(void)
//{
//    SetUpSocket     init(AF_INET, SOCK_STREAM, 0, "5400");
//
//    while (1)
//    {
//        ;
//    }
//    return (0);
//}

/*
int main(void)
{
    std::string file = "GET / HTTP/1.1\r\nHost: example.com\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: 49\r\n\r\nname=FirstName+LastName&email=bsmth%40example.com\r\n";

    HTTP    content(1);
    content.append_content(file.c_str(), file.length() + 1);
    content.parse_input(); 
}
*/

//int main(int ac, char **av)
//{
//    ErrorCodes  pb(atoi(av[1]));
//
//    std::cout << "Error code: " << pb.getcode() << " String value: " << pb.getmessage() << std::endl;
//    return (0);
//}

int main(int ac, char **av)
{
    (void)ac;
    std::string value(av[1]);

    Server ressource(value);
}