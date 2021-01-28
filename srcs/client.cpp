#include "client.hpp"
#define MAXDATASIZE 1000

Client::Client():
     _fd(-1), _read_fd(-1),  _write_fd(-1), _rSet(nullptr), _wSet(nullptr)
{}

Client::Client(int fd, fd_set *readSet, fd_set *writeSet, struct sockaddr_in  client_addr):
     _fd(fd), _read_fd(-1), _write_fd(-1), _rSet(readSet), _wSet(writeSet), _recvInfo(""), _client_addr(client_addr)
{
	_ip = inet_ntoa(client_addr.sin_addr);
	_port = htons(client_addr.sin_port);
    // fcntl(fd, F_SETFL, O_NONBLOCK);
}

Client::~Client()
{}

void Client::setRecvInfo(std::string info)
{
     _recvInfo = info;
}

void Client::setSendInfo(std::string info)
{
     _sendInfo = info;
}
void Client::readFD()
{
     char			buffer[MAXDATASIZE + 1];
     int ret = 0;
     ret = read(_read_fd, buffer, MAXDATASIZE);
     std::cout << buffer << std::endl;
     if (send(_fd, buffer, MAXDATASIZE, 0) == -1)
            perror("send");
}
