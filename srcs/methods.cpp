#include "methods.hpp"

void responseGet(std::vector<Client>::iterator client)
{
	std::string response = client->_request._version;
	std::string		path;
	int ret = 0;
	//check if request file exist
	path = "../www" + client->_request._uri;
	if ((ret = open(path.c_str(), O_RDONLY)) == -1)
	{
		client->setStatus("HTTP/1.1 404 Not Found");
		client->setSendInfo("HTTP/1.1 404 Not Found\n");
		return ;
	}
	// if exits
	client->setReadFD(ret);
}

