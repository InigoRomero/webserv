#ifndef SERVER_HPP
#define SERVER_HPP

#define UNAVAILABLE_TIME  20

#include <string>
#include <iostream>
#include <vector>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h> 
#include <netinet/ip.h>
#include <fcntl.h>
#include <sys/select.h>
#include <unistd.h>
#include <signal.h>

struct methods {
    std::string location;
    std::string name;
    std::string root;
    std::string index;
    std::string cgi;
    std::string cgi_path;
    int max_body;
    int auto_index;
    std::string auth;
};

class Server
{
    private:

    protected:
        //

    public:
        struct sockaddr_in _my_addr;
        std::vector<struct methods> _methods;
        int          _port;
        std::string _error;
        std::string _name;
        std::string _host;
        int         _sockfd;
        
        

    private:
 
    protected:
        //

    public:
        Server();
        Server(int port, std::string error, std::string serverName, std::string );
		~Server();
    
        int start(void);
        fd_set acceptNewClient(fd_set master);
        void setError(const std::string &error);
        void setName(const std::string &name);
        void setHost(const std::string &host);
        void setPort(int port);
        void setMethods(struct methods methods);
};


#endif