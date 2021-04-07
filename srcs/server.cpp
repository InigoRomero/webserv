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
    if ((accept_fd = accept(_sockfd, (struct sockaddr *)&client_addr, &addrlen)) < 0)
    {
        perror("In accept");
        exit(0);
    }
    if (accept_fd > _maxFd)
		_maxFd = accept_fd;
    Client *newClient = new Client(accept_fd, _rSet, _wSet, client_addr);
    _clients.push_back(newClient);
    std::cout << "new Client accepted\n FD : " << accept_fd << std::endl;

    return (1);
}

int		Server::getOpenFd()
{
	int 	nb = 0;
    Client	*client;

	for (std::vector<Client*>::iterator it(_clients.begin()); it != _clients.end(); ++it)
	{
        client = *it;
		nb += 1;
		if (client->_read_fd != -1)
			nb += 1;
		if (client->_write_fd != -1)
			nb += 1;
	}
	return (nb);
}

int Server::refuseConnection()
{
    return (0);
}

std::string tail(std::string const& source, size_t const length) {
  if (length >= source.size()) { return source; }
  return source.substr(source.size() - length);
}

static	int		getpower(int nb, int power)
	{
		if (power < 0)
			return (0);
		if (power == 0)
			return (1);
		return (nb * getpower(nb, power - 1));
	}

static int	fromHexa(const char *nb)
{
	char	base[17] = "0123456789abcdef";
	char	base2[17] = "0123456789ABCDEF";
	int		result = 0;
	int		i;
	int		index;

	i = 0;
	while (nb[i])
	{
		int j = 0;
		while (base[j])
		{
			if (nb[i] == base[j])
			{
				index = j;
				break ;
			}
			j++;
		}
		if (j == 16)
		{
			j = 0;
			while (base2[j])
			{
				if (nb[i] == base2[j])
				{
					index = j;
					break ;
				}
				j++;
			}
		}
		result += index * getpower(16, (strlen(nb) - 1) - i);
		i++;
	}
	return (result);
}

static void getNumber(std::vector<Client*>::iterator it, char *rbuf)
{
    Client		*client = *it;
    int         i = 0;

    while (((rbuf[i] >= '0' && rbuf[i] <= '9') || (rbuf[i] >= 'a' && rbuf[i] <= 'f')))
        i++;
    std::string tmp2 = rbuf;
    if (i > 0 && tmp2.find("\r\n") != std::string::npos)
    {
        char *aux = ft_substr(rbuf, 0, i);
        client->_request->_chucklen = fromHexa(aux);
        if (client->_request->_chucklen != 0)
        {
            char *tmp = ft_substr(rbuf, i + 2, strlen(rbuf));
             memset(client->_request->_rBuf, '\0', BUFFER_SIZE);
            memcpy(client->_request->_rBuf, tmp, strlen(tmp));
            rbuf = client->_request->_rBuf;
            free(tmp);
        }
        free(aux);
    }
}

void Server::parseBody(std::vector<Client*>::iterator it, char *rbuf, size_t bytesToRead)
{
    std::string tmp =   rbuf;
    Client		        *client = *it;
    (void)bytesToRead;
    if ((tail(tmp, 5) != "0\r\n\r\n"))
    {
        if (tmp.find("\r\n") != std::string::npos)
            tmp = tmp.substr(0, tmp.find("\r\n"));
        if ((client->_request->_chuckCont + tmp.size() >= client->_request->_chucklen || tmp.size() > client->_request->_chucklen) && client->_request->_chucklen != 0 )
        {
            client->_request->_req += tmp;
            if (client->_request->_chucklen != 0)
                memset(client->_request->_rBuf, '\0', BUFFER_SIZE);
            client->_request->_chucklen = 0;
            client->_request->_chuckCont = 0;
        }
        else if (strlen(rbuf) == BUFFER_SIZE)
        {
            client->_request->_req += tmp;
            memset(client->_request->_rBuf, '\0', BUFFER_SIZE);
            client->_request->_chuckCont += tmp.size();
        }
        //std::cout << "_chuckCont: [" << client->_request->_chuckCont << "]" << std::endl;
    }
    else
    {
        client->_request->_req += tmp;
        proccessRequest(it);
        memset(client->_request->_rBuf, '\0', BUFFER_SIZE);
        client->_request->_chucklen = 0;
        client->_request->_chuckCont = 0;
    }
    //std::cout << "req [" << client->_request->_req  << "]"<< std::endl;
}

int  Server::readRequest(std::vector<Client*>::iterator it)
 {
    Client		*client = *it;
    char        *rbuf = client->_request->_rBuf;
    int         bytes = strlen(rbuf);
    size_t bytesToRead = BUFFER_SIZE - bytes;
   // std::cout << "HOLA CARACULO"<< std::endl;
    if (client->_request->_chucklen > 0 && bytesToRead > client->_request->_chucklen - client->_request->_chuckCont + 2)
        bytesToRead = client->_request->_chucklen - client->_request->_chuckCont + 2;
    ssize_t     numbytes = read(client->_fd, rbuf + bytes, bytesToRead);  
    numbytes += bytes;
    //std::cout << "bytesToRead [" << bytesToRead  << "]"<< std::endl;
    if (numbytes > 0)
    {
        rbuf[numbytes] = '\0';
        if (client->_request->_body && client->_status == "200 OK")
        {
            //std::cout << "rbuf [" << rbuf  << "]"<< std::endl;
            if(client->_request->_chucklen == 0)
                getNumber(it, rbuf);
            std::cout << "client->_request->_chucklen: " << client->_request->_chucklen << std::endl;
            parseBody(it, rbuf, bytesToRead);
             std::cout << "_req.size() [" << client->_request->_req.size()  << "]"<< std::endl;
            return (0);
        }
        else
        {
            client->_request->_req += rbuf;
            if ((tail(client->_request->_req, 4) == "\r\n\r\n"))
                proccessRequest(it);
            rbuf[0] = '\0';
            return (0);
        }
        client->_lastDate = get_date();
    }
    return (1);
 }

int  Server::writeResponse(std::vector<Client*>::iterator it)
{
    unsigned long	bytes;
    Client		*client = *it;

    
    if (client->_chunkDone/*client->_sendInfo.size() > 10 && client->_write_fd == -1 && client->_read_fd == -1*/)
    {
        client->_sendInfo += "Content-Length: " + std::to_string(client->_contentLength) + "\r\n\r\n";
        if (!client->_request->_bodyIn && client->_chuckBody.size() > 0 && client->_request->_method != "HEAD")
        {
            client->_request->_bodyIn = true;
            client->_sendInfo += client->_chuckBody;
        }
        std::cout << "response size" << client->_sendInfo.size() << std::endl;
        //std::cout << client->_fd << "\nSend info: \n" << client->_sendInfo << std::endl;
        bytes = write(client->_fd, client->_sendInfo.c_str(), client->_sendInfo.size());
        if (bytes < client->_sendInfo.size())
            client->_sendInfo = client->_sendInfo.substr(bytes);
        else
        {
            memset( client->_request->_rBuf, '\0', sizeof(char)*BUFFER_SIZE );
            free(client->_request->_rBuf);
            client->_request->_rBuf = NULL;
            client->_sendInfo.clear();
                FD_CLR(client->_fd, _wSet);
            delete client->_request;
            client->_request = new Request();
            client->_chuckBody.clear();
            client->_chuckBody = "";
            client->_contentLength = 0;
        }
        client->_lastDate = get_date();
        return (0);
    }
    std::cout << "HOLA¿\n";
    //if (client->_write_fd == -1 && client->_read_fd == -1)
    //FD_SET(client->_fd, _rSet);
    return(1);
}

int  Server::proccessRequest(std::vector<Client*>::iterator it)
{
    Client		*client = *it;
    client->setSendInfo("HTTP/1.1");
    client->setStatus("200 OK");
    client->_lastDate = get_date();
    std::cout << "req leng" << client->_request->_req.size() << std::endl;
    if(!client->_request->parseRequest()) // comprobar que nos pasan header -> Host, sin este header http/1.1 responde bad request
    {
        if (client->_request->_body)
            return(0);
        client->setStatus("400 Bad Request");
        sendError(it);
        createHeader(it);
        FD_SET(client->_fd, _wSet);
		return (0);
    }
    getLocationAndMethod(it);
    if (client->_status == "200 OK")
    {
        //std::cout << "Location: " << client->_conf.location << std::endl;
        //std::cout << "Method: " << client->_request->_method << std::endl;
        if (client->_conf.method.find(client->_request->_method) == std::string::npos)
        {
            client->setStatus("405 Not Allowed");
            sendError(it);
            createHeader(it);
            FD_SET(client->_fd, _wSet);
            return (0);
        }
        if (client->_request->_method == "GET")
            responseGet(it);
        else if (client->_request->_method == "POST")
            responsePost(it);
        else if (client->_request->_method == "PUT")
            responsePut(it);
    }
    else
        sendError(it);
    createHeader(it);
    FD_SET(client->_fd, _wSet);
    //FD_CLR(client->_fd, _rSet);
    return 0;
}

void Server::getLocationAndMethod(std::vector<Client*>::iterator it)
{
    std::string aux, aux2;
    Client		*client = *it;
    aux = client->_request->_uri;
    if (aux.size() > 1)
    {
        aux2 = aux.substr(1);
        if (aux2.find("/") != std::string::npos)
            aux = aux2.substr(0, aux2.find("/"));
        else if (aux2.find(".") != std::string::npos)
            aux = aux.substr(0, 1);
        else
            aux = aux2;
    }
    for (std::vector<struct location>::iterator it2 = _locations.begin(); it2 != _locations.end(); it2++)
    {
        if (it2->location.find(aux) != std::string::npos)
        {
            client->_conf = *it2;
            return ;
        }
    }
    client->setStatus("404  Not Found");
}

void Server::sendError(std::vector<Client*>::iterator it)
{
    std::string		path;
    Client		*client = *it;  
    path = _error + "/" + client->_status.substr(0, 3) + ".html";
    client->setPath(path.c_str());
    client->_rFile = ".html";
	client->setReadFd(open(path.c_str(), O_RDONLY));
}

int		Server::getMaxFd()
{
    Client *client;

	for (std::vector<Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
        client = *it;
		if (client->_fd > _maxFd)
			_maxFd = client->_fd;
		if (client->_write_fd > _maxFd)
			_maxFd = client->_write_fd;
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