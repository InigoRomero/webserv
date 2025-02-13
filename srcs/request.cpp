#include "request.hpp"

Request::Request(): _req("")
{
    _headers.insert(std::pair<std::string,std::string>("Accept-Charset", "" ));
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
    _body = false;
    _chucklen = -1;
    _bodyIn = false;
    _cgi = false;
    _rBuf = (char *)malloc(sizeof(char) * (BUFFER_SIZE + 1));
    memset(_rBuf, '\0', sizeof(char)*(BUFFER_SIZE + 1));
}


Request::Request(std::string req): _req(req)
{

}

std::string toLower(std::string src)
{
    std::string tmp;
    for (int i = 0; src[i] != 0; i++)
        tmp.push_back(tolower(src[i]));
    return (tmp);
}

Request::~Request()
{
	_req.clear();
    _headers["body"].clear();
    free(_rBuf);
    _rBuf = NULL; 
}

int Request::parseRequest()
{

	std::vector<std::string> lines;
	size_t pos = 0;
    std::stringstream ss;

    if (_body)
        _headers["body"] = _req;
    else
    {
        std::string tmp = _req;
	    if (_req[0] == '\r')
		    _req.erase(_req.begin());
        if (_req[0] == '\n')
            _req.erase(_req.begin());
        _req = _req.substr(0, _req.find("\r\n\r\n") + 4);
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
        fline.push_back(lines[0]);
        _method = fline[0];
        _uri = fline[1];
        _version = fline[2].substr(0, fline[2].size() - 1);
        if (_avMethods.find(_method) == std::string::npos)
            return 0;
        for (std::vector<std::string>::iterator it = std::next(lines.begin(),1); it != lines.end(); it++)
        {
            size_t pos2 = 0;
            for (std::map<std::string, std::string>::iterator it2 = _headers.begin(); it2 != _headers.end(); it2++)
            {
                if (toLower((*it).substr(0,(*it).find(":"))) == toLower(it2->first))
                {
                    pos2 = (*it).find(":") + 1;
                    while (isspace((*it)[pos2]))
                        pos2++;
                    it2->second = (*it).substr(pos2, (*it).size() - pos2 - 1);
                    break ;
                }
                else
                {
                    pos2 = (*it).find(":") + 1;
                    while (isspace((*it)[pos2]))
                        pos2++;
                    _headers.insert(std::pair<std::string,std::string>((*it).substr(0, (*it).find(":")), (*it).substr(pos2, (*it).size() - pos2 - 1))); 
                }
            }
       }
       if (_headers["Host"] == "")
            return 0;
        tmp = tmp.substr(tmp.find("\r\n\r\n") + 4);
        ss << _headers.find("Content-Length")->second;  
        ss >> pos;
        if (tmp.size() >= pos && _headers.find("Transfer-Encoding")->second != "chunked")
        {
            strcpy(_rBuf, tmp.c_str());
            _body = true;
            return (0);
        }
        strcpy(_rBuf, tmp.c_str());
        if (_method == "POST" || _method == "PUT")
        {
            _body = true;
            _req.clear();
            return(0);
        }
    }
    return (1);
}

void Request::setRequest(std::string req)
{
	_req = req;
}

void Request::execCGI(Client &client)
{
    char **args = NULL;
    char **env = NULL;
    int ret;
    int fd[2];
    std::string path;

    close(client._read_fd);
    client._read_fd = -1; 

    if (!(args = (char**)malloc(sizeof(char *) * 3)))
        return ;
    args[0] = strdup(client._conf.cgi_path.c_str()); 
    args[1] = strdup(client._path.c_str()); 
    args[2] = NULL;
    env = setEnv(client);

    client._tmp_fd = open("./www/temp_file", O_WRONLY | O_CREAT, 0666);
    pipe(fd);
	if (client._request->_method == "GET")
		close(fd[1]);
    if (!(client._cgi_pid = fork()))
    {
        close(fd[1]);
        dup2(fd[0], 0);
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
        memset(env[i], '\0', strlen(env[i]));
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
	envMap["SCRIPT_NAME"] = client._conf.cgi_path;
	envMap["SERVER_NAME"] = "webserv";
	envMap["SERVER_PORT"] = "80";
	if (client._request->_headers.find("Authorization") != client._request->_headers.end())
	{
		pos = client._request->_headers["Authorization"].find(" ");
		envMap["AUTH_TYPE"] = client._request->_headers["Authorization"].substr(0, pos);
		envMap["REMOTE_USER"] = client._request->_headers["Authorization"].substr(pos + 1);
		envMap["REMOTE_IDENT"] = client._request->_headers["Authorization"].substr(pos + 1);
	}
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
	while (it != envMap.end())
	{
		env[i] = strdup((it->first + "=" + it->second).c_str());
		++i;
		++it;
	}
	env[i] = NULL;
	return (env);
}

void		Request::parseCGIResult(Client &client)
{
    (void)client;
	size_t			pos;
	std::string		headers;
	std::string		key;
	std::string		value;

	if (client._chuckBody.find("\r\n\r\n") == std::string::npos)
		return ;
	headers = client._chuckBody.substr(0, client._chuckBody.find("\r\n\r\n") + 1);
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
		client._request->_headers[key] = value;
		key.clear();
		value.clear();
		if (headers[pos] == '\r')
			pos++;
		if (headers[pos] == '\n')
			pos++;
	}
	pos = client._chuckBody.find("\r\n\r\n") + 4;
	client._chuckBody = client._chuckBody.substr(pos);
	client._contentLength = client._chuckBody.size();
}