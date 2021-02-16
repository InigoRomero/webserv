#include "methods.hpp"

void responseGet(std::vector<Client>::iterator client, Server serv)
{
	std::string response = client->_request->_version;
	std::string		path;
	int ret = 0;
	size_t pos;

	path = client->_request->_uri;
	for (std::vector<struct methods>::iterator it = serv._methods.begin(); it != serv._methods.end(); it++)
	{
		if ((pos = client->_request->_uri.find(it->location)) != std::string::npos)
		{
			if (pos + 1 < client->_request->_uri.size())
				path =  it->root + "/"+ client->_request->_uri.substr(pos + 1, std::string::npos);
			else
				path = it->root + "/"+ it->index;
		}
	}
	if ((pos = path.find_last_of(".")) != std::string::npos)
		client->setRFile(path.substr(pos, std::string::npos));
	if ((ret = open(path.c_str(), O_RDONLY)) == -1)
	{
		client->setStatus("HTTP/1.1 400 Bad Request");
		client->setSendInfo("HTTP/1.1 400 Bad Request\r\n");
		return ;
	}
	// if exits
	client->setPath(path.c_str());
	client->setReadFd(ret);
}

void responsePost(std::vector<Client>::iterator client, Server serv)
{
	(void)serv;
	client->_request->parseBody((*client));
	std::cout << "send Info: " <<client->_sendInfo << std::endl;
}


void createHeader(std::vector<Client>::iterator client, Server serv)
{
	std::map<std::string, std::string> 	headers;
	client->setSendInfo(client->_sendInfo + "Sever: "+ serv._name + "/1.0.0\r\n");
	client->setSendInfo(client->_sendInfo + "Date: " + get_date() + "\r\n");
	client->setSendInfo(client->_sendInfo + "Last-Modified: " + getLastModified(client->_path)+ "\r\n"); //date de archivo requested by client
	client->setSendInfo(client->_sendInfo + "Content-Type: " + getDataType(client->_rFile) + "\r\n");
	//client->setSendInfo(client->_sendInfo + "Content-Type: text/html\r\n");
}

std::string getDataType(std::string fileExt)
{
	if (fileExt == ".txt")
		return ("text/plain");
	else if (fileExt == ".bin")
		return ("application/octet-stream");
	else if (fileExt == ".jpeg")
		return ("image/jpeg");
	else if (fileExt == ".jpg")
		return ("image/jpeg");
	else if (fileExt == ".html")
		return ("text/html");
	else if (fileExt == ".htm")
		return ("text/html");
	else if (fileExt == ".png")
		return ("image/png");
	else if (fileExt == ".bmp")
		return ("image/bmp");
	else if (fileExt == ".pdf")
		return ("application/pdf");
	else if (fileExt == ".tar")
		return ("application/x-tar");
	else if (fileExt == ".json")
		return ("application/json");
	else if (fileExt == ".css")
		return ("text/css");
	else if (fileExt == ".js")
		return ("pplication/javascript");
	else if (fileExt == ".mp3")
		return ("audio/mpeg");
	else if (fileExt == ".avi")
		return ("video/x-msvideo");
	else
		return ("error");
}

std::string getLastModified(std::string path)
{
	char		buf[32 + 1];
	int			ret;
	struct tm	*tm;
	struct stat	file_info;

	if (lstat(path.c_str(), &file_info) == -1)
		return ("");
	tm = localtime(&file_info.st_mtime);
	ret = strftime(buf, 32, "%a, %d %b %Y %T %Z", tm);
	buf[ret] = '\0';
	return (buf);
}
