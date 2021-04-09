#ifndef SERVER_HPP
#define SERVER_HPP

#define UNAVAILABLE_TIME  20

#include "methods.hpp"
#include "get_next_line.hpp"

class Server
{
    private:

    protected:
        //

    public:
        struct sockaddr_in          _my_addr;
        std::vector<struct location> _locations;
        std::map<std::string, std::string> 	_headers;
        std::vector<Client*>         _clients;
        int                         _port;
        std::string                 _error;
        std::string                 _name;
        std::string                 _host;
        std::string                 _conf;
        int                         _maxFd;
        int                         _sockfd;
		fd_set					    *_readSet;
		fd_set					    *_writeSet;
		fd_set					    *_rSet;
		fd_set					    *_wSet;
        
        

    private:
 
    protected:
        //

    public:
        Server();
        Server(int port, std::string error, std::string serverName, std::string host);
		~Server();
    
        int     start(fd_set *readSet, fd_set *writeSet, fd_set *rSet, fd_set *wSet);
        int     acceptNewClient();
        int     refuseConnection();
        void    setError(const std::string &error);
        void    setName(const std::string &name);
        void    setHost(const std::string &host);
        void    setConf(const std::string &host);
        void    setPort(int port);
        void    setLocations(struct location methods);
        int     readRequest(std::vector<Client*>::iterator it);
        int     writeResponse(std::vector<Client*>::iterator it);
        int     proccessRequest(std::vector<Client*>::iterator it);
        void    sendError(std::vector<Client*>::iterator it);
        void    getLocationAndMethod(std::vector<Client*>::iterator it);
        void    parseBody(std::vector<Client*>::iterator it, char *rbuf);
        int     getMaxFd();
        int		getOpenFd();
};

#endif