#include "client.hpp"
#define MAXDATASIZE 10000

Client::Client():
     _fd(-1), _read_fd(-1),  _write_fd(-1), _rSet(nullptr), _wSet(nullptr), _request(new Request())
{}

Client::Client(int fd, fd_set *readSet, fd_set *writeSet, struct sockaddr_in  client_addr):
     _fd(fd), _read_fd(-1), _write_fd(-1) ,_rSet(readSet), _wSet(writeSet),  _client_addr(client_addr), _request(new Request()), _status(""),  _tmp_fd(-1), _cgi_pid(-1)
{
	_ip = inet_ntoa(client_addr.sin_addr);
	_port = htons(client_addr.sin_port);
     _contentLength = 0;
     _chuckBody = "";
     fcntl(_fd, F_SETFL, O_NONBLOCK);
     FD_SET(fd, _rSet);
	//FD_SET(fd, _wSet);
}

Client::~Client()
{
	
}

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
     char			buffer[BUFFER_SIZE + 1];
     int ret = 0,   status = 0;

     if (_cgi_pid != -1)
	{
		if (waitpid(_cgi_pid, (int *)&status, (int)WNOHANG) == 0)
			return ;
		else if (WEXITSTATUS(status) == 1)
		{
               close(_tmp_fd);
               _tmp_fd = -1;
               _cgi_pid = -1;
               unlink("./www/temp_file");
               close(_read_fd);
               _contentLength = _chuckBody.size();
               setReadFd(-1);
               _request->_headers["body"] = "Error with cgi\n";
               return ;
		}
	}
     ret = read(_read_fd, buffer, BUFFER_SIZE);
     if (ret >= 0)
		buffer[ret] = '\0';
     std::string	tmp(buffer, ret);
    // std::cout << "HE lido de archivo: " << tmp << std::endl;
     _chuckBody += tmp;
     memset(buffer, '\0', sizeof(char)*BUFFER_SIZE );
     if (ret == 0)
	{
          close(_read_fd);
          unlink("./www/temp_file");
          _contentLength = _chuckBody.size();
          setReadFd(-1);
          if (_cgi_pid != -1)
          {
               close(_tmp_fd);
               _tmp_fd = -1;
               _cgi_pid = -1;
               _request->parseCGIResult(*this);
          }
     }
}

void Client::writeFd()
{
     int ret = 0;

     ret = write(_write_fd, _request->_headers["body"].c_str(), _request->_headers["body"].size());
     //std::cout << "HE escrito en el cgi: " << _request->_headers["body"] << std::endl;
     _write_fd = -1;
     close(_write_fd);
     _chunkDone = true;
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