#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <iostream>
#include <string.h>
#include <vector>
#include <map>
#include <string>
/*
struct headers {
    std::string acceptCharsets;
    std::string acceptLanguage;
    std::string allow;
    std::string authorization;
    std::string contentLanguage;
    std::string contentLength;
    std::string contentLocation;
    std::string contentType;
    std::string date;
    std::string Host;
    std::string lastModified;
    std::string location;
    std::string referer;
    std::string tetryAfter;
    std::string server;
    std::string transferEncoding;
    std::string userAgent;
    std::string WWWAuthenticate;
};*/

class Request
{
private:
	/* data */
public:
	Request();
	Request(std::string req);
	~Request();

    std::string                 _req;
   // std::vector<std::string>    _headersAllowed;
    std::vector<std::string>    _methods;
    std::map<std::string, std::string>          _headers; 
    bool                        _validate;

    void setRequest(std::string req);
	void parseRequest();
	void parseHeaders();
    void validateHeader(std::vector<std::string> reqL);
    void validateRequest();
};

#endif