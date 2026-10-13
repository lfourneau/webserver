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
    SetUpSocket     init(AF_INET, SOCK_STREAM, 0, "54000");

    return (0);
}

