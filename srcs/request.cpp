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
	size_t pos = 0, found = 0;

	while ((pos = _req.find('\n')) != std::string::npos) {
    	lines.push_back(_req.substr(0, pos));
    	_req.erase(0, pos + 1);
	}
    validateHeader(lines);
    if (_validate) //rembember to change !_validate
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

void Request::setRequest(std::string req)
{
	_req = req;
}