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

class Client
{

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
		std::string         _recvInfo;
        struct sockaddr_in  _client_addr;
        std::string         _ip;
        char*         _sendInfo;

        void setRecvInfo(std::string info);
        void setSendInfo(char *info);
};

#endif