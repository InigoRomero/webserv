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

void Server::parseBody(std::vector<Client*>::iterator it)
{
    Client		        *client = *it;
    size_t              pos;
    std::string         tmp = client->_request->_rBuf;
    std::string         aux;
    std::stringstream   stream;

    if (client->_request->_headers.find("Content-Length")->second != "")
        parseNoChunked(it);
    else if (client->_request->_headers.find("Transfer-Encoding")->second == "chunked")
    {
        while (tmp.size() > 0)
        {
            if ((pos = tmp.find("\r\n")) != std::string::npos)
            {
                if (client->_chunkFinal == false)
                {
                    if (pos == 0)
                    {
                        tmp = tmp.erase(0, 2);
                        continue;
                    }
                    client->_chunkFinal = true;
                    if (tmp.substr(0, pos) == "0")
                        client->_request->_chucklen = 0;
                    else
                    {
                        stream << std::hex << tmp.substr(0, pos);
                        stream >> client->_request->_chucklen;
                    }
                    tmp = tmp.substr(pos + 2);
                    if (tmp == "")
                        memset(client->_request->_rBuf, '\0', BUFFER_SIZE + 1);
                }
                else
                {
                    client->_chunkFinal = false;
                    aux = tmp.substr(0, pos);
                    client->_request->_req += aux;
                    client->_chuckCont += aux.size();
                    tmp = tmp.substr(pos + 2);
                    if (client->_request->_chucklen == 0)
                        proccessRequest(it);
                    memset(client->_request->_rBuf, '\0', BUFFER_SIZE + 1);
                    client->_request->_chucklen = -1;
                    client->_chuckCont = 0;
                }
            }
            else
            {
                if (((client->_chuckCont + (int)tmp.size()) > client->_request->_chucklen) && client->_request->_chucklen >= 0)
                {
                    client->_request->_req += tmp.substr(0, client->_request->_chucklen - client->_chuckCont);
                    tmp = tmp.substr(client->_request->_chucklen - client->_chuckCont);
                    strcpy(client->_request->_rBuf, tmp.c_str());
                    client->_chuckCont = 0;
                    break ;
                }
                else if (client->_request->_chucklen > 0)
                {
                    client->_request->_req += tmp;
                    client->_chuckCont += tmp.size();
                    memset(client->_request->_rBuf, '\0', BUFFER_SIZE + 1);
                    break ;
                }
                else
                {
                    strcpy(client->_request->_rBuf, tmp.c_str());
                    break ;
                }
            } 
        }
    }
    else
    {
        //std::cout << "hello2\n";
        client->setStatus("400 Bad Request");
        sendError(it);
        createHeader(it);
        FD_SET(client->_fd, _wSet);
        client->_chunkDone = true;
    }
}


void Server::parseNoChunked(std::vector<Client*>::iterator it)
{
    Client		        *client = *it;
    std::string         tmp = client->_request->_rBuf;
    std::stringstream ss;
    size_t num;

    client->_request->_req += tmp;
    tmp.clear();
    memset(client->_request->_rBuf, '\0', BUFFER_SIZE + 1);
    ss << client->_request->_headers.find("Content-Length")->second;  
    ss >> num; 
    if (client->_request->_req.size() >= num)
        proccessRequest(it);
}

int  Server::readRequest(std::vector<Client*>::iterator it)
 {
    Client		*client = *it;
    int         bytes = strlen(client->_request->_rBuf);
    size_t bytesToRead = BUFFER_SIZE - bytes;
    ssize_t     numbytes = read(client->_fd, client->_request->_rBuf + bytes, bytesToRead);  
    numbytes += bytes;

    if (numbytes > 0)
    {
        client->_request->_rBuf[numbytes] = '\0';
        if (client->_request->_body)
        {
            parseBody(it);
            return (0);
        }
        else
        {
            client->_request->_req += client->_request->_rBuf;
            if (client->_request->_req.find("\r\n\r\n") != std::string::npos)
                proccessRequest(it);
            //std::cout << "mam:" << client->_request->_rBuf << std::endl;
            if (!client->_request->_body)
                memset(client->_request->_rBuf, '\0', BUFFER_SIZE + 1);
            else if (strcmp(client->_request->_rBuf, "") != 0)
                parseBody(it);
            return (0);
        }
        client->_lastDate = get_date();
    }
    if (numbytes <= 0)
        client->_kick = true;
    return (1);
 }

int  Server::writeResponse(std::vector<Client*>::iterator it)
{
    unsigned long	bytes;
    Client		*client = *it;

    if (client->_chunkDone)
    {
        if (!client->_request->_bodyIn)
            client->_sendInfo += "Content-Length: " + std::to_string(client->_chuckBody.size()) + "\r\n\r\n";
        if (!client->_request->_bodyIn && client->_request->_method != "HEAD")
        {
            //std::cout << "RESPONSE [" << client->_sendInfo.substr(0, 100) << "] \n";
            client->_request->_bodyIn = true;
            client->_sendInfo += client->_chuckBody;
        }
        //std::cout << "sendinfo:\n" << client->_sendInfo << std::endl;
        bytes = write(client->_fd, client->_sendInfo.c_str(), client->_sendInfo.size());
        if (bytes < client->_sendInfo.size())
            client->_sendInfo = client->_sendInfo.substr(bytes);
        else if (bytes > 0)
        {
            memset( client->_request->_rBuf, '\0', sizeof(char)*BUFFER_SIZE );
            client->_sendInfo.clear();
            FD_CLR(client->_fd, _wSet);
            delete client->_request;
            client->_request = new Request();
            client->_standBy = false;
            client->_chuckBody.clear();
            client->_contentLength = 0;
        }
        if (bytes <= 0)
            client->_kick = true;
        client->_lastDate = get_date();
        return (0);
    }
    return(1);
}

int  Server::proccessRequest(std::vector<Client*>::iterator it)
{
    Client		*client = *it;
    client->_chunkDone = false;
    int ret;

    if (!client->_sendInfo.size())
        client->setSendInfo("HTTP/1.1");
    client->setStatus("200 OK");
    client->_lastDate = get_date();

    if(! (ret = client->_request->parseRequest())) // comprobar que nos pasan header -> Host, sin este header http/1.1 responde bad request
    {
        if (client->_request->_body)
            return(0);
        client->setStatus("400 Bad Request");
        sendError(it);
        createHeader(it);
        FD_SET(client->_fd, _wSet);
        client->_chunkDone = true;
		return (0);
    }
    if (ret == 2)
        proccessRequest(it);
    getLocationAndMethod(it);
    if (client->_status == "200 OK")
    {
        if (client->_request->_body && client->_conf.max_body > 0 && client->_conf.max_body < (int)client->_request->_headers["body"].size())
        {
            client->setStatus("413 Bad Request");
            sendError(it);
            createHeader(it);
            FD_SET(client->_fd, _wSet);
            client->_chunkDone = true;
            return (0);
        }
        if (client->_conf.method.find(client->_request->_method) == std::string::npos)
        {
            client->setStatus("405 Not Allowed");
            sendError(it);
            createHeader(it);
            FD_SET(client->_fd, _wSet);
            client->_chunkDone = true;
            return (0);
        }
        if (client->_request->_method == "GET")
            responseGet(it);
        else if (client->_request->_method == "POST")
            responsePost(it);
        else if (client->_request->_method == "PUT")
            responsePut(it);
        //else if (client->_request->_method == "HEAD")
        //    responseHead(it);
        else if (client->_request->_method == "DELETE")
            responseDelete(it);
    }
    else
        sendError(it);
    createHeader(it);
    FD_SET(client->_fd, _wSet);
    client->_standBy = true;
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
            aux = aux.substr(0, aux2.find("/") + 1);
        else if (aux2.find(".") != std::string::npos)
            aux = aux.substr(0, 1);
    }
    size_t found;
    for (std::vector<struct location>::iterator it2 = _locations.begin(); it2 != _locations.end(); it2++)
    {
        if ((found = it2->location.find(aux) != std::string::npos) && aux.size() == it2->location.size())
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

void	Server::send503()
{
	std::string		response;
	int				ret = 0, fd = -1;
    struct sockaddr_in  client_addr;
    socklen_t             addrlen;
    addrlen = sizeof(client_addr);

    if ((fd = accept(_sockfd, (struct sockaddr *)&client_addr, &addrlen)) < 0)
    {
        perror("In accept");
        exit(0);
    }

	response = "HTTP/1.1 503\r\n";
    response +=  "Sever: webserv/1.0.0\r\n";
	response += "Date: " + get_date() + "\r\n"; 
	response += "Retry-After: 25\r\n\r\n";
	response += "503\r\n\r\n";

	ret = write(fd, response.c_str(), response.size());
	if (ret >= -1)
	{
		close(fd);
		FD_CLR(fd, _wSet);
	}
}

//seters
void	Server::setError(const std::string &error) { this->_error = error; }

void	Server::setName(const std::string &name) { this->_name = name; }

void	Server::setHost(const std::string &host) { this->_host = host; }

void	Server::setConf(const std::string &conf) { this->_conf = conf; }

void	Server::setPort(int port) { this->_port = port; }

void	Server::setLocations(struct location methods) { this->_locations.push_back(methods); }