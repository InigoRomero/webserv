#include "server.hpp"

Server::Server(std::string serverName, int port):
    _sockfd(-1), _name(serverName)
{
    _my_addr.sin_family = AF_INET;
    _my_addr.sin_port = port;
    _my_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&_my_addr, sizeof(_my_addr));
}

Server::~Server()
{}

int Server::start(void)
{
    errno = 0;
	
    // socket
	if ((_sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		std::cout << "Error: " << "Server::start -> socket(): " << std::string(strerror(errno)) << std::endl;
        return (0);
    }
    else
        std::cout << _name << " port:" << std::to_string(_my_addr.sin_port) << " socket= " << std::to_string(_sockfd) << std::endl;
    bind(_sockfd, (struct sockaddr *)&_my_addr, sizeof _my_addr);
    int yes=1;

    // lose the pesky "Address already in use" error message
    if (setsockopt(_sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof yes) == -1) {
        perror("setsockopt");
        return(0);
    }
     if (listen(_sockfd, 10) == -1)
	{
		perror("listen");
        return (0);
    }
    else
		std::cout << _name << " port:" << std::to_string(_my_addr.sin_port) << " lister OK " << std::endl;
    return (1);
}

int Server::acceptNewClient(void)
{
    int accept_fd = 0;
    struct sockaddr_in	client_addr;
    int addrlen = sizeof(client_addr);

    bzero(&client_addr, sizeof(client_addr));
}