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

int Server::start(fd_set *readSet, fd_set *writeSet, fd_set *rSet, fd_set *wSet)
{
    _readSet = readSet;
	_writeSet = writeSet;
	_wSet = wSet;
	_rSet = rSet;
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
    /*
    if (fcntl(_sockfd, F_SETFL, O_NONBLOCK) == -1)
	{
		perror("fcntl");
        return (0);
    }*/
    return (1);
}

int Server::acceptNewClient(fd_set *readSet, fd_set *writeSet)
{
    int                 accept_fd = 0;
    struct sockaddr_in  client_addr;
    ssize_t             addrlen;


    memset(&client_addr, 0, sizeof(struct sockaddr));
    addrlen = sizeof client_addr;
    if ((accept_fd = accept(_sockfd, (struct sockaddr *)&client_addr, (socklen_t*)&addrlen)) < 0)
    {
        perror("In accept");
        exit(0);
    }
    Client newClient = Client(accept_fd, readSet, writeSet, client_addr);
    _clients.push_back(newClient);
    FD_SET(accept_fd, readSet);
    //FD_CLR(accept_fd, readSet);
    std::cout << "new Client accepted\n";

    return (1);
}

 int  Server::readRequest(std::vector<Client>::iterator it)
 {
    ssize_t             numbytes;
    char                buf[10000];

    if ((numbytes = recv(it->_fd, buf, 9999, 0)) == -1) {
        perror("recv");
        exit(1);
    }
    buf[numbytes] = '\0';
    std::string str(buf);
    //std::cout << buf << std::endl;
    it->_request.setRequest(str); 
    return(1);
 }

int  Server::writeResponse(std::vector<Client>::iterator it)
{
    std::cout << "sending mlaplana passwords... " << std::endl;
    char char_array[it->_sendInfo.size()];
    strcpy(char_array, it->_sendInfo.c_str());
    if (send(it->_fd, char_array, it->_sendInfo.size(), 0) == -1)
        perror("send");
    return(1);
}

int  Server::proccessRequest(std::vector<Client>::iterator it)
{
    it->setSendInfo("HTTP/1.1 200 OK\r\n");
    it->setStatus("HTTP/1.1 200 OK");
    if(!it->_request.parseRequest())
    {
        it->setSendInfo("HTTP/1.1 400 bad Request\r\n");
        it->setStatus("400");
        sendError(it);
    }
    if (it->_request._method == "GET")
        responseGet(it, (*this));
    if (it->_status != "HTTP/1.1 200 OK")
        sendError(it);
   // FD_SET(it->_fd, _writeSet);
    createHeader(it, (*this));
    return 0;
}

void Server::sendError(std::vector<Client>::iterator it)
{
    std::string		path;

    size_t pos = it->_status.find(" ");
    path = _error + "/" + it->_status.substr(pos + 1, 3) + ".html";
    it->setPath(path.c_str());
	it->setReadFd(open(path.c_str(), O_RDONLY));
}

//seters
void	Server::setError(const std::string &error) { this->_error = error; }

void	Server::setName(const std::string &name) { this->_name = name; }

void	Server::setHost(const std::string &host) { this->_host = host; }

void	Server::setConf(const std::string &conf) { this->_conf = conf; }

void	Server::setPort(int port) { this->_port = port; }

void	Server::setMethods(struct methods methods) { this->_methods.push_back(methods); }