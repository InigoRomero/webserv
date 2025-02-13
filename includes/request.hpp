#ifndef REQUEST_HPP
#define REQUEST_HPP
#define BUFFER_SIZE 32768

#include <iostream>
#include <string.h>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include "client.hpp"

class Client;

class Request
{
    private:
        /* data */
    public:
        Request();
        Request(std::string req);
        ~Request();

        char                                *_rBuf;
        std::string                         _req;
        std::string                         _avMethods;
        std::string                         _method;
        std::string                         _uri;
        std::string                         _version;
        std::map<std::string, std::string>  _headers;
        unsigned int                        _bodyLen;
        int                                 _chucklen;
        bool                                _body;
        bool                                _bodyIn;
        bool                                _cgi;

        void setRequest(std::string req);
        int parseRequest();
        void parseHeaders();
        void validateRequest();
        void handleGet();
        void execCGI(Client &client);
        //void setRbuf(char *req);
        char **setEnv(Client &client);
        void parseCGIResult(Client &client);
};

#endif