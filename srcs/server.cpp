#include "server.hpp"
#define MAXDATASIZE 1000

Server::Server():
     _locations(), _port(-1),  _error(""), _name(""), _host("")
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
		//std::cout << "Error: " << "Server::start -> socket(): " << std::string(strerror(errno)) << std::endl;
        return (0);
    }
    int yes = 1;
    if (setsockopt(_sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof (int)) == -1) {
        perror("setsockopt");
        return(0);
    }
    if (bind(_sockfd, (struct sockaddr *)&_my_addr, sizeof _my_addr) < 0)
    {
        perror("In bind");
        return(0);
    }

    if (listen(_sockfd, 256) < 0)
    {
        perror("listen");
        return(0);
    }
    if (fcntl(_sockfd, F_SETFL, O_NONBLOCK) == -1)
	{
		perror("fcntl");
        return (0);
    }
    FD_SET(_sockfd, _rSet);
    _maxFd = _sockfd;
    return (1);
}

int Server::acceptNewClient()
{
    int                 accept_fd = 0;
    struct sockaddr_in  client_addr;
    socklen_t             addrlen;


    memset(&client_addr, 0, sizeof(struct sockaddr));
    addrlen = sizeof(client_addr);
    std::cout << "SERVER FD IN ACEPT" << _sockfd << std::endl;
    if ((accept_fd = accept(_sockfd, (struct sockaddr *)&client_addr, &addrlen)) < 0)
    {
        perror("In accept");
        exit(0);
    }
    if (accept_fd > _maxFd)
		_maxFd = accept_fd;
    Client newClient = Client(accept_fd, _rSet, _wSet, client_addr);

    _clients.push_back(newClient);
   // FD_SET(accept_fd, readSet);
    //FD_CLR(accept_fd, readSet);
   // std::cout << "new Client accepted\n";

    return (1);
}

 int  Server::readRequest(std::vector<Client>::iterator it)
 {
    ssize_t             numbytes;
    int bytes;

    numbytes = 0;
    std::cout << "Entro a leer" << std::endl;
    bytes = strlen(it->_request->_rBuf);
    if ((numbytes = read(it->_fd, it->_request->_rBuf  + bytes, BUFFER_SIZE - bytes)) == -1) {
        perror("read");
        exit(1);
    }
    it->_request->_rBuf [numbytes + bytes] = '\0';
    std::cout << "\nLEIDO DEL CLIENTE:\n*****\n" << it->_request->_rBuf << "\n*****\n" << std::endl;
    if ((strstr(it->_request->_rBuf , "\r\n\r\n") != NULL && strstr(it->_request->_rBuf , "chunked") == NULL) || (strstr(it->_request->_rBuf , "0\r\n\r\n") != NULL && strstr(it->_request->_rBuf , "chunked") != NULL))
    {
        std::string str1 = it->_request->_rBuf;
        it->_request->setRequest(str1);
        //  std::cout << "\nLEIDO DEL CLIENTE:\n*****\n" << it->_request->_rBuf << "\n*****\n" << std::endl;
        return(0);
    }
    return (1);
 }

int  Server::writeResponse(std::vector<Client>::iterator it)
{
    unsigned long	bytes;

    it->_sendInfo += "Content-Length: " + std::to_string(it->_contentLength) + "\r\n\r\n";
    if (it->_chuckBody.size() > 0)
        it->_sendInfo += it->_chuckBody;
    std::cout << "\n\nSend info: \n" << it->_sendInfo << std::endl;
    bytes = write(it->_fd, it->_sendInfo.c_str(), it->_sendInfo.size());
    if (bytes < it->_sendInfo.size())
        it->_sendInfo = it->_sendInfo.substr(bytes);
    else
    {
        memset( it->_request->_rBuf, '\0', sizeof(char)*BUFFER_SIZE );
        free(it->_request->_rBuf);
        it->_request->_rBuf = NULL;
        it->_sendInfo.clear();
        delete it->_request;
        it->_request = new Request();
    }
    it->_lastDate = get_date();
    return(1);
}

int  Server::proccessRequest(std::vector<Client>::iterator it)
{
    it->setSendInfo("HTTP/1.1");
    it->setStatus("200 OK");
    if(!it->_request->parseRequest()) // comprobar que nos pasan header -> Host, sin este header http/1.1 responde bad request
    {
        it->setStatus("400 Bad Request");
        sendError(it);
        createHeader(it);
        FD_SET(it->_fd, _wSet);
		return (0);
    }
    getLocationAndMethod(it);
  //  std::cout << "Location: " << it->_conf.location << std::endl;
  //  std::cout << "Method: " << it->_request->_method << std::endl;
    if (it->_conf.method.find(it->_request->_method) == std::string::npos)
	{
		it->setStatus("405 Not Allowed");
        sendError(it);
        createHeader(it);
        FD_SET(it->_fd, _wSet);
		return (0);
	}
    if (it->_request->_method == "GET")
        responseGet(it);
    else if (it->_request->_method == "POST")
        responsePost(it, (*this));
    else if (it->_request->_method == "PUT")
        responsePut(it);

  //  std::cout << "status: " << it->_status << std::endl;
    if (it->_status != "200 OK")
        sendError(it);
    createHeader(it);
    FD_SET(it->_fd, _wSet);
    //FD_CLR(it->_fd, _rSet);
    return 0;
}

void Server::getLocationAndMethod(std::vector<Client>::iterator it)
{
    std::string aux;

    aux = it->_request->_uri;
    if (aux.size() > 1)
    {
       aux = aux.substr(1);
        aux = aux.substr(0, aux.find("/"));
    }
    for (std::vector<struct location>::iterator it2 = _locations.begin(); it2 != _locations.end(); it2++)
    {
        if (it2->location.find(aux) != std::string::npos)
        {
            it->_conf = *it2;
            return ;
        }
    }
    it->setStatus("400 Bad Request");
}

void Server::sendError(std::vector<Client>::iterator it)
{
    std::string		path;

    path = _error + "/" + it->_status.substr(0, 3) + ".html";
    it->setPath(path.c_str());
	it->setReadFd(open(path.c_str(), O_RDONLY));
   // std::cout << "readfd: " << it->_read_fd << std::endl;
    //std::cout << "path: " << path << std::endl;
}

int		Server::getMaxFd()
{
	for (std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->_fd > _maxFd)
			_maxFd = it->_fd;
		if (it->_write_fd > _maxFd)
			_maxFd = it->_write_fd;
	}
	return (_maxFd);
}

//seters
void	Server::setError(const std::string &error) { this->_error = error; }

void	Server::setName(const std::string &name) { this->_name = name; }

void	Server::setHost(const std::string &host) { this->_host = host; }

void	Server::setConf(const std::string &conf) { this->_conf = conf; }

void	Server::setPort(int port) { this->_port = port; }

void	Server::setLocations(struct location methods) { this->_locations.push_back(methods); }