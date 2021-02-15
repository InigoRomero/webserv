#ifndef REQUEST_HPP
#define REQUEST_HPP
#define BUFFER_SIZE 32768

#include <iostream>
#include <string.h>
#include <vector>
#include <map>
#include <string>
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

        std::string                         _req;
        std::vector<std::string>            _avMethods;
        std::string                         _method;
        std::string                         _uri;
        std::string                         _version;
        std::map<std::string, std::string>  _headers;
        unsigned int                        _bodyLen;
        bool                                _validate;

        void setRequest(std::string req);
        int parseRequest();
        void parseHeaders();
        void validateHeader(std::vector<std::string> reqL);
        void validateRequest();
        void handleGet();
        void execCGI();
        void parseBody(Client &client);
};

#endif