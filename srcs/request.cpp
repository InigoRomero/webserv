#include "request.hpp"

Request::Request(): _req("")
{
    _headers.insert(std::pair<std::string,std::string>("Accept-Charsets", "" ));
    _headers.insert(std::pair<std::string,std::string>("Accept-Language", "")); 
    _headers.insert(std::pair<std::string,std::string>("Allow", "")); 
    _headers.insert(std::pair<std::string,std::string>("Authorization", "")); 
    _headers.insert(std::pair<std::string,std::string>("Content-Language", "")); 
    _headers.insert(std::pair<std::string,std::string>("Content-Length", "")); 
    _headers.insert(std::pair<std::string,std::string>("Content-Location", "")); 
    _headers.insert(std::pair<std::string,std::string>("Content-Type", "")); 
    _headers.insert(std::pair<std::string,std::string>("Date", "")); 
    _headers.insert(std::pair<std::string,std::string>("Host", "")); 
    _headers.insert(std::pair<std::string,std::string>("Last-Modified", "")); 
    _headers.insert(std::pair<std::string,std::string>("Location", "")); 
    _headers.insert(std::pair<std::string,std::string>("Referer", "")); 
    _headers.insert(std::pair<std::string,std::string>("Retry-After", "")); 
    _headers.insert(std::pair<std::string,std::string>("Server", "")); 
    _headers.insert(std::pair<std::string,std::string>("Transfer-Encoding", "")); 
    _headers.insert(std::pair<std::string,std::string>("User-Agent", "")); 
    _headers.insert(std::pair<std::string,std::string>("WWW-Authenticate", ""));
    _headers.insert(std::pair<std::string,std::string>("body", ""));
    _avMethods = "GET|POST|PUT|HEAD|CONNECT|OPTIONS|TRACE|DELETE";
    _rBuf = (char *)malloc(sizeof(char) * (BUFFER_SIZE + 1));
    memset(_rBuf, '\0', sizeof(char)*BUFFER_SIZE );
}

Request::Request(std::string req): _req(req)
{

}

Request::~Request()
{
  
}

int Request::parseRequest()
{

	std::vector<std::string> lines;
	size_t pos = 0, found = 0;
    //std::cout << "request:\n*****\n" << _req << "\n*****\n";
    _headers["body"] = _req.substr(_req.find("\r\n\r\n") + 4, std::string::npos);
    //std::cout << "Body:" << _headers["body"] <<  std::endl;
    std::string aux;
    if ((pos = _headers["body"].find("\r\n")) != std::string::npos)
        _headers["body"] = _headers["body"].substr(pos+2, std::string::npos);
	while ((pos = _headers["body"].find("\r\n")) != std::string::npos) {
        aux += _headers["body"].substr(0, pos);
        _headers["body"] = _headers["body"].substr(pos+2, std::string::npos);
        if ((pos = _headers["body"].find("\r\n")) != std::string::npos)
            _headers["body"] = _headers["body"].substr(pos+2, std::string::npos);
	}
    if (aux != "")
        _headers["body"] = aux;
    //std::cout << "Body2:" << _headers["body"] <<  std::endl;
    //std::cout << "Body:" << _headers["body"] <<  std::endl;  
    _req = _req.substr(0, _req.find("\r\n\r\n"));
	while ((pos = _req.find('\n')) != std::string::npos) {
    	lines.push_back(_req.substr(0, pos));
        _req = _req.substr(pos+1);
	}
    std::string::iterator it = lines[0].begin();
    //eliminar espacios repetidos
    while (isspace(*it))
        it = lines[0].erase(it);
    while (it != lines[0].end())
    {
        if (isspace(*it) && isspace(*(std::next(it))))
            it = lines[0].erase(it);
        else
            it++;
    }
    std::vector<std::string> fline;
	while ((pos = lines[0].find(' ')) != std::string::npos) {
    	fline.push_back(lines[0].substr(0, pos));
    	lines[0].erase(0, pos + 1);
	}
    fline.push_back(lines[0]); // pushear fuera si hay espacio despues de http/1.1 ?
    _method = fline[0];
    //std::cout << "Method of the client: " << _method << std::endl;
    _uri = fline[1];
    _version = fline[2];
    if (_avMethods.find(_method) == std::string::npos)
        return 0;
    //take all the headers we have to take
    for (std::vector<std::string>::iterator it = std::next(lines.begin(),1); it != lines.end(); it++)
    {
        for (std::map<std::string, std::string>::iterator it2 = _headers.begin(); it2 != _headers.end(); it2++)
        {
            if ((found = (*it).find(it2->first)) != std::string::npos)
            {
                it2->second = (*it).substr((*it).find(":") + 1, (*it).size());
                break ;
            }
        }
    }
    return (1);
}

void Request::parseBody(Client &client)
{
    //std::cout << "rbuf: \n" << client._request->_rBuf << std::endl;
    if (client._request->_headers.find("Content-Length") != client._request->_headers.end())
    {   
        // GET BODY
        unsigned int	bytes;
        // if the llega el content Length
        client._request->_bodyLen = atoi(client._request->_headers["Content-Length:"].c_str());
        if (client._request->_bodyLen < 0)
        {
            client.setSendInfo("400 Bad Request Error");
            return ;
        }
        bytes = strlen(client._request->_req.c_str()); // creo que _req se queda vacio al leerla y hacerle substr
        if (bytes >= client._request->_bodyLen)
        {
            memset(client._request->_rBuf+ client._request->_bodyLen , 0, BUFFER_SIZE - client._request->_bodyLen);
            client._sendInfo += client._request->_rBuf;
            client._request->_bodyLen = 0;
        }
        else
            client.setStatus("400 Bad Request");
    }
    if(client._request->_headers["Transfer-Encoding:"].find("chunked") != std::string::npos)
    {
        //std::cout << "Estoy en DECHUNKBODY" << std::endl;
        //DECHUNKBODY
        if (strstr(client._request->_req.c_str(), "\r\n")  && client._chunkFound == false)
        {
            client._request->_bodyLen = findLen(client);
            if (client._request->_bodyLen == 0)
		        client._chunkDone = true;
		    else
			    client._chunkFound = true;
        }
        if (client._chunkFound == true)
		    fillBody(client);
        if (client._chunkDone)
        {
            memset(client._request->_rBuf, 0, BUFFER_SIZE + 1);
            client._chunkDone = false;
            client._chunkFound = false;
            return ;
        }
    }
    else
        client.setStatus("400 Bad Request");
    
}

void			Request::fillBody(Client &client)
{
	std::string		tmp;

	tmp = client._request->_rBuf;
	if (tmp.size() > client._request->_bodyLen)
	{
		client._sendInfo+= tmp.substr(0, client._request->_bodyLen);
		tmp = tmp.substr(client._request->_bodyLen + 1);
		memset(client._request->_rBuf, 0, BUFFER_SIZE + 1);
		strcpy(client._request->_rBuf , tmp.c_str());
		client._request->_bodyLen = 0;
		client._chunkFound = false;
	}
	else
	{
		client._sendInfo += tmp;
		client._request->_bodyLen -= tmp.size();
		memset(client._request->_rBuf, 0, BUFFER_SIZE + 1);
	}
}


int				Request::findLen(Client &client)
{
	std::string		to_convert;
	int				len;

	to_convert = client._request->_req;
	to_convert = to_convert.substr(0, to_convert.find("\r\n"));
	while (to_convert[0] == '\n')
		to_convert.erase(to_convert.begin());
	if (to_convert.size() == 0)
		len = 0;
	else
		len = strlen(to_convert.c_str());
	len = strlen(to_convert.c_str());

	return (len);
}


void Request::setRequest(std::string req)
{
	_req = req;
}
/*
void Request::setRbuf(char *req)
{
	_rBuf = req;
}*/

void Request::execCGI(Client &client)
{
    char **args = NULL;
    char **env = NULL;
    int ret;
    int fd[2];
    std::string path;

    close(client._read_fd); //why
    client._read_fd = -1; //idk
    //cond
    if (!(args = (char**)malloc(sizeof(char *) * 3)))
        return ;
    if (client._conf.location.size() < client._request->_uri.size())
	{
		if (client._request->_uri.find(".") == std::string::npos)
			path =  client._conf.root + "/"+ client._request->_uri.substr(client._conf.location.size(), std::string::npos) + "/" + client._conf.index;
		else
			path =  client._conf.root + "/"+ client._request->_uri.substr(client._conf.location.size(), std::string::npos);
	}
	else
		path = client._conf.root + "/"+ client._conf.index;
    args[0] = strdup(client._conf.cgi_path.c_str()); // req->location->cgi_root || php_root // cgi path del requested location
    args[1] = strdup(path.c_str()); // req->file
    args[2] = NULL;
    env = setEnv(client);

    client._tmp_fd = open("./www/temp_file", O_WRONLY | O_CREAT, 0666);
    pipe(fd);
    if (!(client._cgi_pid = fork()))
    {
        close(fd[1]);
        dup2(fd[0], 0);  //why
        dup2(client._tmp_fd, 1);
        errno = 0;
		if ((ret = execve(client._conf.cgi_path.c_str(), args, env)) == -1)
		{
			std::cerr << "Error with CGI: " << strerror(errno) << std::endl;
			exit(1);
		}
    }
    else
    {
        close(fd[0]);
        client._write_fd = fd[1];
        client._read_fd = open("./www/temp_file", O_RDONLY);
    }
    free(args[0]);
    free(args[1]);
    free(args);
    int i = 0;
    while (env[i])
    {
        free(env[i]);
        ++i;
    }
    free(env);
}

char			**Request::setEnv(Client &client)
{
	char	**env;
    std::map<std::string, std::string> 				envMap;
	size_t											pos;

	envMap["GATEWAY_INTERFACE"] = "CGI/1.1";
	envMap["SERVER_PROTOCOL"] = "HTTP/1.1";
	envMap["SERVER_SOFTWARE"] = "webserv";
	envMap["REQUEST_URI"] = client._request->_uri;
	envMap["REQUEST_METHOD"] = client._request->_method;
	envMap["REMOTE_ADDR"] = client._ip;
	envMap["PATH_INFO"] = client._request->_uri;
	envMap["PATH_TRANSLATED"] = client._conf.cgi_path;
	envMap["CONTENT_LENGTH"] = std::to_string(_headers["body"].size());

	if (client._request->_uri.find('?') != std::string::npos)
		envMap["QUERY_STRING"] = client._request->_uri.substr(client._request->_uri.find('?') + 1);
	else
		envMap["QUERY_STRING"];
	if (client._request->_headers.find("Content-Type") != client._request->_headers.end())
		envMap["CONTENT_TYPE"] = client._request->_headers["Content-Type"];
	//if (client._conf.exe != client.conf.end())
//		envMap["SCRIPT_NAME"] = client.conf["exec"];
//	else
	envMap["SCRIPT_NAME"] = client._conf.cgi_path;
	/*if (client._port)
	{
		envMap["SERVER_NAME"] = "webserv";
		envMap["SERVER_PORT"] = client._port;
	}
	else*/
	envMap["SERVER_PORT"] = "80";
	if (client._request->_headers.find("Authorization") != client._request->_headers.end())
	{
		pos = client._request->_headers["Authorization"].find(" ");
		envMap["AUTH_TYPE"] = client._request->_headers["Authorization"].substr(0, pos);
		envMap["REMOTE_USER"] = client._request->_headers["Authorization"].substr(pos + 1);
		envMap["REMOTE_IDENT"] = client._request->_headers["Authorization"].substr(pos + 1);
	}
	//if (client._conf.find("php") != client.conf.end() && client._request->.uri.find(".php") != std::string::npos)
	//	envMap["REDIRECT_STATUS"] = "200";

	std::map<std::string, std::string>::iterator b = client._request->_headers.begin();
	while (b != client._request->_headers.end())
	{
        if(b->second.size() > 0 && b->first != "body")
		    envMap["HTTP_" + b->first] = b->second;
		++b;
	}
	env = (char **)malloc(sizeof(char *) * (envMap.size() + 1));
	std::map<std::string, std::string>::iterator it = envMap.begin();
	int i = 0;
    //std::cout << "ENV" << std::endl;
	while (it != envMap.end())
	{
		env[i] = strdup((it->first + "=" + it->second).c_str());
       // std::cout << env[i] << std::endl;
		++i;
		++it;
	}
	env[i] = NULL;
	return (env);
}

void		Request::parseCGIResult(Client &client)
{
    (void)client;
    /*
	size_t			pos;
	std::string		headers;
	std::string		key;
	std::string		value;

	if (client.res.body.find("\r\n\r\n") == std::string::npos)
		return ;
	headers = client.res.body.substr(0, client.res.body.find("\r\n\r\n") + 1);
	pos = headers.find("Status");
	if (pos != std::string::npos)
	{
		client.res.status_code.clear();
		pos += 8;
		while (headers[pos] != '\r')
		{
			client.res.status_code += headers[pos];
			pos++;
		}
	}
	pos = 0;
	while (headers[pos])
	{
		while (headers[pos] && headers[pos] != ':')
		{
			key += headers[pos];
			++pos;
		}
		++pos;
		while (headers[pos] && headers[pos] != '\r')
		{
			value += headers[pos];
			++pos;
		}
		client.res.headers[key] = value;
		key.clear();
		value.clear();
		if (headers[pos] == '\r')
			pos++;
		if (headers[pos] == '\n')
			pos++;
	}
	pos = client.res.body.find("\r\n\r\n") + 4;
	client.res.body = client.res.body.substr(pos);
	client.res.headers["Content-Length"] = std::to_string(client.res.body.size());*/
}