#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <sys/wait.h>
#include <string.h>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <string>
#include <map>
#include "request.hpp"

class Request;

class Client
{
    friend class Request;
    private:
    protected:
        //

    public:
        Client();
        Client(int fd, fd_set *readSet, fd_set *writeSet, struct sockaddr_in  client_addr);
        ~Client();
		int			        _fd;
		int			        _read_fd;
		int			        _write_fd;
        int                 _port;
		fd_set		        *_rSet;
		fd_set		        *_wSet;
        struct sockaddr_in  _client_addr;
        std::string         _ip;
        std::string         _sendInfo;
        std::string         _path;
        std::string         _rFile;
        Request             *_request;
        std::string         _status;
        pid_t               _cgi_pid;


        void setSendInfo(std::string info);
        void setReadFd(int fd);
        void setStatus(std::string status);
        void setPath(std::string path);
        void setRFile(std::string file);
        void readFd();
        void writeFd();
};

#endif