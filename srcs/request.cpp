#include "request.hpp"

Request::Request(): _req(""), _validate(false)
{
    _headers.insert(std::pair<std::string,std::string>("Accept-Charsets:", "" ));
    _headers.insert(std::pair<std::string,std::string>("Accept-Language:", "")); 
    _headers.insert(std::pair<std::string,std::string>("Allow:", "")); 
    _headers.insert(std::pair<std::string,std::string>("Authorization:", "")); 
    _headers.insert(std::pair<std::string,std::string>("Content-Language:", "")); 
    _headers.insert(std::pair<std::string,std::string>("Content-Length:", "")); 
    _headers.insert(std::pair<std::string,std::string>("Content-Location:", "")); 
    _headers.insert(std::pair<std::string,std::string>("Content-Type:", "")); 
    _headers.insert(std::pair<std::string,std::string>("Date:", "")); 
    _headers.insert(std::pair<std::string,std::string>("Host:", "")); 
    _headers.insert(std::pair<std::string,std::string>("Last-Modified:", "")); 
    _headers.insert(std::pair<std::string,std::string>("Location:", "")); 
    _headers.insert(std::pair<std::string,std::string>("Referer:", "")); 
    _headers.insert(std::pair<std::string,std::string>("Retry-After:", "")); 
    _headers.insert(std::pair<std::string,std::string>("Server:", "")); 
    _headers.insert(std::pair<std::string,std::string>("Transfer-Encoding:", "")); 
    _headers.insert(std::pair<std::string,std::string>("User-Agent:", "")); 
    _headers.insert(std::pair<std::string,std::string>("WWW-Authenticate:", ""));
    _headers.insert(std::pair<std::string,std::string>("body", ""));
    this->_avMethods.push_back("GET");
    this->_avMethods.push_back("POST");
    this->_avMethods.push_back("PUT");
    this->_avMethods.push_back("HEAD");
    this->_avMethods.push_back("CONNECT");
    this->_avMethods.push_back("OPTIONS");
    this->_avMethods.push_back("TRACE");
    this->_avMethods.push_back("PATCH");
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
	size_t pos = 0, found = 0, aux;

	while ((pos = _req.find('\n')) != std::string::npos) {
    	lines.push_back(_req.substr(0, pos));
        aux = pos;
        _req = _req.substr(pos+1);
	}
    validateHeader(lines);
    if (!_validate)
        return(0);
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
    _uri = fline[1];
    _version = fline[2];
    if (_method == "POST" || _method == "PUT")
        lines.push_back(_req.substr(aux - 1, std::string::npos));
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
    _headers["body"] = lines.back();
    return (1);
}

void Request::validateHeader(std::vector<std::string> reqL)
{
    //check method
    size_t found = 0;

    for (std::vector<std::string>::iterator it = _avMethods.begin(); it != _avMethods.end(); it++)
    {
        if ((found = reqL[0].find((*it))) != std::string::npos)
        {
            _validate = true;
            break ;
        }
    }
}

void Request::parseBody(Client &client)
{
   // std::cout << "rbuf: " << client._request->_rBuf << std::endl;
  //  if (client._request->_headers.find("Content-Length") != client._request->_headers.end())
   // {   
        
        // GET BODY
        unsigned int	bytes;
        // if the llega el content Length
        client._request->_bodyLen = atoi(client._request->_headers["Content-Length"].c_str());
        if (client._request->_bodyLen < 0)
        {
            client.setSendInfo("400 Bad Request Error");
            return ;
        }
        
        bytes = strlen(client._request->_req.c_str()); // creo que _req se queda vacio al leerla y hacerle substr
        if (bytes >= client._request->_bodyLen)
        {
           // memset(client._request->_rBuf+ client._request->_bodyLen , 0, BUFFER_SIZE - client._request->_bodyLen);
            client._sendInfo += client._request->_rBuf;
            client._request->_bodyLen = 0;
        }
        else
            client.setStatus("HTTP/1.1 400 Bad Request");
  //  }
    if(client._request->_headers["Transfer-Encoding"] == "chunked")
    {
        std::cout << "Estoy en DECHUNKBODY" << std::endl;
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
        client.setStatus("HTTP/1.1 400 Bad Request");
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

void Request::setRbuf(char *req)
{
	_rBuf = req;
}

void Request::execCGI()
{
    /*char **args = NULL;
    char **env = NULL;
    int ret;
*/
}