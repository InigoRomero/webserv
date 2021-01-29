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
    this->_methods.push_back("GET");
    this->_methods.push_back("POST");
    this->_methods.push_back("PUT");
    this->_methods.push_back("HEAD");
    this->_methods.push_back("CONNECT");
    this->_methods.push_back("OPTIONS");
    this->_methods.push_back("TRACE");
    this->_methods.push_back("PATCH");
}

Request::Request(std::string req): _req(req)
{

}

Request::~Request()
{

}

void Request::parseRequest()
{
	std::cout << "Server recived\n";
	//std::cout << _req << std::endl;
	std::vector<std::string> lines;
	size_t pos = 0, found = 0;

	while ((pos = _req.find('\n')) != std::string::npos) {
    	lines.push_back(_req.substr(0, pos));
    	std::cout << lines.back() << std::endl;
    	_req.erase(0, pos + 1);
	}
    validateHeader(lines);
    std::cout << "Validated: " << _validate << std::endl;
    std::string::iterator it = lines[0].begin();
    while (isspace(*it))
        it = lines[0].erase(it);
    while (it != lines[0].end())
    {
        if (isspace(*it) && isspace(*(std::next(it))))
            it = lines[0].erase(it);
        else
            it++;
    }
    std::cout << lines[0] << std::endl;
    //pos = lines[0].find(' ');
    //std::string method = lines[0].substr(0, pos);
    //std::string uri = lines[0].substr(pos + 1, ' ');
    
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
    for(std::map<std::string, std::string>::iterator it= _headers.begin();
    it != _headers.end(); ++it)
    {
        std::cout << it->first << " - " << it->second <<"\n";
    }
    /*lines[0].erase(std::remove_if(lines[0].begin(), lines[0].end(), isspace), lines[0].end());
	std::cout << lines[0];
	std::string method = lines[0].substr(0, lines[0].find('/'));
	std::string ver = lines[0].substr(lines[0].find("HTTP/1.1"), std::string::npos);
	if (ver != "HTTP/1.1")
		exit(1);
	std::string uri = lines[0].substr(lines[0].find('/'), lines[0].find("HTTP/1.1"));
    */
	//std::string method = lines[0].
}

void Request::validateHeader(std::vector<std::string> reqL)
{
    //check method
    size_t found = 0;

    for (std::vector<std::string>::iterator it = _methods.begin(); it != _methods.end(); it++)
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