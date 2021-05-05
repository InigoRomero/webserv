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

std::string ReplaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}

void print_helper(std::string tmp, char c)
{
    if (tmp.size() > 20)
    {
        std::cout << "tmp" << c <<  "|" << tmp.substr(0,10) << "..." << tmp.substr(tmp.size() - 10) << "|" << std::endl;
    }
    else
        std::cout << "tmp" << c <<  "|" << tmp << "|" <<std::endl;
}

void Server::parseBody(std::vector<Client*>::iterator it)
{
    Client		        *client = *it;
    size_t              pos;
    std::string         tmp = client->_request->_rBuf;
    std::string         aux;
    std::stringstream   stream;

    while (tmp.size() > 0)
    {
        /*print_helper(tmp, '1');
        if (!client->_chunkFinal)
            std::cout << "bool:false" << std::endl;
        else
            std::cout << "bool:true" << std::endl;*/
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
                //std::cout << "tmp1:" << tmp.substr(0, pos) << std::endl;
                //print_helper(tmp.substr(0, pos), '2');
                if (tmp.substr(0, pos) == "0")
                    client->_request->_chucklen = 0;
                else
                {
                    stream << std::hex << tmp.substr(0, pos);
                    stream >> client->_request->_chucklen;
                }
                //std::cout << "len:" << client->_request->_chucklen << std::endl;
                tmp = tmp.substr(pos + 2);
                if (tmp == "")
                {
                    memset(client->_request->_rBuf, '\0', BUFFER_SIZE + 1);
                    //print_helper(tmp.substr(0, pos), 'h');
                }
            }
            else
            {
                client->_chunkFinal = false;
                aux = tmp.substr(0, pos);
                client->_request->_req += aux;
                //std::cout << "totalsize1:" << client->_request->_req.size() << std::endl;
                client->_chuckCont += aux.size();
                tmp = tmp.substr(pos + 2);
                if (client->_request->_chucklen == 0)
                {
                    tmp.clear();
                    //std::cout << "totalsize2:" << client->_request->_req.size() << std::endl;
                    proccessRequest(it);
                    //std::cout << "queloque\n";
                    memset(client->_request->_rBuf, '\0', BUFFER_SIZE + 1);
                    client->_request->_chucklen = -1;
                    client->_chuckCont = 0;
                    break ;
                }
                //std::cout << "tmp2:" << tmp << std::endl;
                //print_helper(tmp, '3');
                memset(client->_request->_rBuf, '\0', BUFFER_SIZE + 1);
                client->_request->_chucklen = -1;
                client->_chuckCont = 0;
            }
        }
        else
        {
            //std::cout << "tmp3:" << tmp << std::endl;
            //print_helper(tmp, 'E');
            if (tmp.size() == BUFFER_SIZE)
            {
                if (((client->_chuckCont + (int)tmp.size()) > client->_request->_chucklen) && client->_chuckCont != 0)
                {
                    //std::cout << "tmp4:" << tmp << std::endl;
                    //print_helper(tmp, '4');
                    client->_request->_req += tmp.substr(0, client->_request->_chucklen - client->_chuckCont);
                    //std::cout << "totalsize3:" << client->_request->_req.size() << std::endl;
                    tmp = tmp.substr(client->_request->_chucklen - client->_chuckCont);
                    //std::cout << "tmp5:" << tmp << std::endl;
                    //print_helper(tmp, '5');
                    strcpy(client->_request->_rBuf, tmp.c_str());
                    tmp.clear();
                    client->_chuckCont = 0;
                }
                else
                {
                    //std::cout << "tmp6:" << tmp << std::endl;
                    //print_helper(tmp, '6');
                    client->_request->_req += tmp;
                    //std::cout << "totalsize4:" << client->_request->_req.size() << std::endl;
                    client->_chuckCont += tmp.size();
                    tmp.clear();
                    memset(client->_request->_rBuf, '\0', BUFFER_SIZE + 1);  
                }
            }
            else
            {
                strcpy(client->_request->_rBuf, tmp.c_str());
                tmp.clear();
            }
        } 
    }
}

int  Server::readRequest(std::vector<Client*>::iterator it)
 {
    Client		*client = *it;
    int         bytes = strlen(client->_request->_rBuf);
    size_t bytesToRead = BUFFER_SIZE - bytes;
    ssize_t     numbytes = read(client->_fd, client->_request->_rBuf + bytes, bytesToRead);  
    numbytes += bytes;

    //std::cout << "numbytes:" << numbytes << std::endl;
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
            if (!client->_request->_body)
                memset(client->_request->_rBuf, '\0', BUFFER_SIZE + 1);
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

    if (client->_chunkDone)
    {
        if (!client->_request->_bodyIn)
            client->_sendInfo += "Content-Length: " + std::to_string(client->_chuckBody.size()) + "\r\n\r\n";
        if (!client->_request->_bodyIn && client->_request->_method != "HEAD")
        {
            //std::cout << "RESPONSE [" << client->_sendInfo.substr(0, 100) << "] \n";
            //std::cout << "response:" << client->_sendInfo << std::endl;
            client->_request->_bodyIn = true;
            client->_sendInfo += client->_chuckBody;
        }
        //std::cout << "sendinfo:\n" << client->_sendInfo << std::endl;
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
    return(1);
}

int  Server::proccessRequest(std::vector<Client*>::iterator it)
{
    Client		*client = *it;
    client->_chunkDone = false;

    if (!client->_sendInfo.size())
        client->setSendInfo("HTTP/1.1");
    client->setStatus("200 OK");
    client->_lastDate = get_date();

    if(!client->_request->parseRequest()) // comprobar que nos pasan header -> Host, sin este header http/1.1 responde bad request
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
        else if (client->_request->_method == "HEAD")
            responseHead(it);
        else if (client->_request->_method == "DELETE")
            responseDelete(it);
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