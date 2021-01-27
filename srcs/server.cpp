#include "server.hpp"
#define MAXDATASIZE 1000

Server::Server():
     _methods(), _port(-1),  _error(""), _name(""), _host("")
{
    bzero(&_my_addr, sizeof(_my_addr));
}

Server::Server(int port, std::string error, std::string serverName, std::string host):
      _port(port),  _error(error), _name(serverName), _host(host)
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
    int yes = 1;
    if (setsockopt(_sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof yes) == -1) {
        perror("setsockopt");
        return(0);
    }
    if (bind(_sockfd, (struct sockaddr *)&_my_addr, sizeof _my_addr)<0)
    {
        perror("In bind");
        return(0);
    }

    if (listen(_sockfd, 10) < 0)
    {
        perror("listen");
        return(0);
    }
   /* if (fcntl(_sockfd, F_SETFL, O_NONBLOCK) == -1)
	{
		perror("fcntl");
        return (0);
    }*/
    return (1);
}

fd_set Server::acceptNewClient(fd_set master)
{
    int accept_fd = 0;
    struct sockaddr_in	client_addr;
    ssize_t addrlen;
    ssize_t numbytes;
    char buf[MAXDATASIZE];
   memset(&client_addr, 0, sizeof(struct sockaddr));

    printf("server: waiting for connections...\n");
    addrlen = sizeof client_addr;
    if ((accept_fd = accept(_sockfd, (struct sockaddr *)&client_addr, (socklen_t*)&addrlen))<0)
    {
        perror("In accept");
        exit(0);
    }
        printf("selectserver: new connection");
       if ((numbytes = recv(accept_fd, buf, MAXDATASIZE-1, 0)) == -1) {
            perror("recv");
            exit(1);
        }
        buf[numbytes] = '\0';
        printf("client: received '%s'\n",buf);
        if (send(accept_fd, "Hello, world!", 13, 0) == -1)
                perror("send");
        close(accept_fd);  // parent doesn't need this*/
   // FD_SET(_sockfd, &master);
    return (master);
}

//seters
void	Server::setError(const std::string &error)
{ 
    this->_error = error; 
}

void	Server::setName(const std::string &name)
{ 
    this->_name = name; 
}

void	Server::setHost(const std::string &host)
{ 
    this->_host = host; 
}

void	Server::setPort(int port)
{ 
    this->_port = port; 
}

void	Server::setMethods(struct methods methods)
{ 
    this->_methods.push_back(methods); 
}