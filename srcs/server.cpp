#include "server.hpp"

Server::Server(std::string serverName, int port):
    _sockfd(-1) , _port(port),  _name(serverName)
{
    bzero(&_my_addr, sizeof(_my_addr));
}

Server::~Server()
{}

int Server::start(void)
{
    errno = 0;
    // socket
    _my_addr.sin_family = AF_INET;
    _my_addr.sin_port = htons( _port );
    _my_addr.sin_addr.s_addr = INADDR_ANY;
	if ((_sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		std::cout << "Error: " << "Server::start -> socket(): " << std::string(strerror(errno)) << std::endl;
        return (0);
    }
    if (bind(_sockfd, (struct sockaddr *)&_my_addr, sizeof _my_addr)<0)
    {
        perror("In bind");
        return(0);
    }
    int yes = 1;
    if (setsockopt(_sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof yes) == -1) {
        perror("setsockopt");
        return(0);
    }

    if (listen(_sockfd, 10) < 0)
    {
        perror("In listen");
        exit(EXIT_FAILURE);
    }
    return (1);
}

int Server::acceptNewClient(void)
{
    int accept_fd = 0;
    //struct sockaddr_in	client_addr;
    int addrlen = sizeof(_my_addr);

    //bzero(&client_addr, sizeof(client_addr));

    while(1)
    {
        printf("\n+++++++ Waiting for new connection ++++++++\n\n");
        if ((accept_fd = accept(_sockfd, (struct sockaddr *)&_my_addr, (socklen_t*)&addrlen))<0)
        {
            perror("In accept");
            return(0);
        }
        
        char buffer[30000] = {0};
        read( accept_fd , buffer, 30000);
        printf("%s\n",buffer );
    // send(new_socket, "hola", 4, 0); //escribir con send
        write(accept_fd , "hola", 4);
        printf("------------------Hello message sent-------------------\n");
        close(accept_fd);
    }
    return (1);
}