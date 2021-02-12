#include "client.hpp"
#define MAXDATASIZE 10000

Client::Client():
     _fd(-1), _read_fd(-1),  _write_fd(-1), _rSet(nullptr), _wSet(nullptr), _request(new Request())
{}

Client::Client(int fd, fd_set *readSet, fd_set *writeSet, struct sockaddr_in  client_addr):
     _fd(fd), _read_fd(-1), _write_fd(-1), _rSet(readSet), _wSet(writeSet), _client_addr(client_addr), _request(new Request()), _status("HTTP/1.1 200 OK")
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

void Client::setReadFd(int fd)
{
     _read_fd = fd;
}

void Client::readFd()
{
     
     char			buffer[MAXDATASIZE + 1];
     int ret = 0;

     ret = read(_read_fd, buffer, MAXDATASIZE);
     buffer[ret] = '\0';
     //comproba
     _sendInfo += "Content-Length: " + std::to_string(strlen(buffer)) + "\r\n\r\n";
     _sendInfo += buffer;
     close(_read_fd);
     setReadFd(-1);
}

void Client::writeFd()
{
     int ret = 0;

     ret = write(_write_fd, _request->_body.c_str(), _request->_body.length()); // .size()??
     
}

void Client::setStatus(std::string status)
{
     _status = status;
}

void Client::setPath(std::string path)
{
     _path = path;
}

void Client::setRFile(std::string file)
{
     _rFile = file;
}