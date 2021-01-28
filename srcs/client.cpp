#include "client.hpp"

Client::Client():
     _fd(-1), _read_fd(-1),  _write_fd(-1), _rSet(nullptr), _wSet(nullptr)
{}

Client::Client(int fd, fd_set *readSet, fd_set *writeSet, std::string recvInfo):
     _fd(fd), _rSet(readSet), _wSet(writeSet), _recvInfo(recvInfo)
{}

Client::~Client()
{}