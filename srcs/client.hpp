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
        Client(int fd, fd_set *readSet, fd_set *writeSet, std::string recvInfo);
        ~Client();
		int			_fd;
		int			_read_fd;
		int			_write_fd;
		fd_set		*_rSet;
		fd_set		*_wSet;
		std::string _recvInfo;
};

#endif