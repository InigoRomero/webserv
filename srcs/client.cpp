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
     _chunkFinal = false;
     _chuckCont = 0;
     _chunkDone = false;
     _error = false;
     fcntl(_fd, F_SETFL, O_NONBLOCK);
     FD_SET(fd, _rSet);
}

Client::~Client()
{
	if (_fd != -1)
	{
		close(_fd);
		FD_CLR(_fd, _rSet);
		FD_CLR(_fd, _wSet);
	}
	if (_read_fd != -1)
	{
		close(_read_fd);
		FD_CLR(_read_fd, _rSet);
	}
	if (_write_fd != -1)
	{
		close(_write_fd);
		FD_CLR(_write_fd, _wSet);	
	}
	if (_tmp_fd != -1)
	{
		close(_tmp_fd);
		unlink("./www/temp_file");
	}
     delete _request;
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
     char			buffer[32678 + 1];
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
     ret = read(_read_fd, buffer, 32678);
     if (ret >= 0)
		buffer[ret] = '\0';
     std::string	tmp(buffer, ret);
     _chuckBody += tmp;
     memset(buffer, '\0', sizeof(char)*32678 );
     if (ret == 0)
	{
          close(_read_fd);
          unlink("./www/temp_file");
          _contentLength = _chuckBody.size();
          setReadFd(-1);
          _chunkDone = true;
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
     _write_fd = -1;
     close(_write_fd);
     if (_cgi_pid == -1)
          _chunkDone = true;
}

void Client::setStatus(std::string status)
{
     _status = status;
}

void Client::setPath()
{
     if (_request->_method == "PUT")
          _path =  _conf.root + "/"+ _request->_uri.substr(_conf.location.size(), std::string::npos);
     else if (_conf.location.size() < _request->_uri.size())
     {
          if (_request->_uri.find(".") == std::string::npos)
               _path =  _conf.root + _request->_uri.substr(_conf.location.size(), std::string::npos) + "/" + _conf.index;
          else
               _path =  _conf.root + "/" + _request->_uri.substr(_conf.location.size(), std::string::npos);
     }
     else
          _path = _conf.root + "/"+ _conf.index;
}

void Client::setRFile(std::string file)
{
     _rFile = file;
}