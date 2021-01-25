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
class Server
{
    private:

    protected:
        //

    public:
        struct sockaddr_in _my_addr;
        int _sockfd;
        int _port;
        std::string _name;

    private:
        Server();

    protected:
        //

    public:
        Server(std::string serverName, int port);
		~Server();
    
        int start(void);
        
        int acceptNewClient(void);
};


#endif