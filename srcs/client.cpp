#include "client.hpp"
#define MAXDATASIZE 1000

Client::Client():
     _fd(-1), _read_fd(-1),  _write_fd(-1), _rSet(nullptr), _wSet(nullptr), _request(Request())
{}

Client::Client(int fd, fd_set *readSet, fd_set *writeSet, struct sockaddr_in  client_addr):
     _fd(fd), _read_fd(-1), _write_fd(-1), _rSet(readSet), _wSet(writeSet), _client_addr(client_addr), _request(Request())
{
	_ip = inet_ntoa(client_addr.sin_addr);
	_port = htons(client_addr.sin_port);
    // fcntl(fd, F_SETFL, O_NONBLOCK);
}

Client::~Client()
{}

void Client::setSendInfo(std::string info)
{
     _sendInfo = info;
}

void Client::setReadFD(int fd)
{
     _read_fd = fd;
}

void Client::readFD()
{
     
     char			buffer[MAXDATASIZE + 1];
     int ret = 0;

     ret = read(_read_fd, buffer, MAXDATASIZE);
     buffer[ret] = '\0';
     _sendInfo += buffer;
     setReadFD(-1);
}
