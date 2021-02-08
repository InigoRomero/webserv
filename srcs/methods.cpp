#include "methods.hpp"

void responseGet(std::vector<Client>::iterator client, Server serv)
{
	std::string response = client->_request._version;
	std::string		path;
	int ret = 0;

	path = ".."+client->_request._uri;
	for (std::vector<struct methods>::iterator it = serv._methods.begin(); it != serv._methods.end(); it++)
		if (client->_request._uri == it->location)
			path = "." + it->root + "/"+ it->index;
	std::cout << path << std::endl;
	if ((ret = open(path.c_str(), O_RDONLY)) == -1)
	{
		client->setStatus("HTTP/1.1 404 Not Found");
		client->setSendInfo("HTTP/1.1 404 Not Found\n");
		return ;
	}
	// if exits
	client->setReadFD(ret);
}

