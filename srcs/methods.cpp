#include "methods.hpp"

void responseGet(std::vector<Client>::iterator client)
{
	std::string response = client->_request->_version;
	std::string		path;
	int ret = 0;
	size_t pos;

	if (client->_conf.location.size() < client->_request->_uri.size())
	{
		if (client->_request->_uri.find(".") == std::string::npos)
			path =  client->_conf.root + "/"+ client->_request->_uri.substr(client->_conf.location.size(), std::string::npos) + "/" + client->_conf.index;
		else
			path =  client->_conf.root + "/"+ client->_request->_uri.substr(client->_conf.location.size(), std::string::npos);
	}
	else
		path = client->_conf.root + "/"+ client->_conf.index;
	if ((pos = path.find_last_of(".")) != std::string::npos)
		client->setRFile(path.substr(pos, std::string::npos));
	
	std::cout << "PATH: " << path << std::endl;
	if ((ret =  open(path.c_str(), O_RDONLY)) == -1)
	{
		client->setStatus("404 Not Found");
		return ;
	}
	// if exits
	client->setPath(path.c_str());
	client->setReadFd(ret);
}

void responsePost(std::vector<Client>::iterator client)
{
	size_t pos;
	std::string ext;
	std::string	path;

	path =  client->_conf.root + "/"+ client->_request->_uri.substr(client->_conf.location.size(), std::string::npos);
	if ((pos = path.find_last_of(".")) != std::string::npos)
		ext = path.substr(pos, std::string::npos);
	if ((client->_conf.cgi != ""  && client->_conf.cgi == ext))
	{
		client->_request->execCGI(*client);
	}
	else
	{
		if ((open(path.c_str(), O_RDONLY)) == -1)
			client->setStatus("201 OK");
		client->_write_fd = open(path.c_str(), O_CREAT|O_WRONLY|O_NONBLOCK, 0666);
	}

	path =  client->_conf.root + "/"+ client->_request->_uri.substr(client->_conf.location.size(), std::string::npos);
	//si el archivo no existia y se ha creado devolver 201, si ya existia y ha sido modificado 200 o contenido vacio 204
	if ((open(path.c_str(), O_RDONLY)) == -1)
		client->setStatus("201 OK");
	client->_write_fd = open(path.c_str(), O_CREAT|O_WRONLY|O_NONBLOCK, 0666);
}

void responsePut(std::vector<Client>::iterator client)
{
	std::string	path;

	path =  client->_conf.root + "/"+ client->_request->_uri.substr(client->_conf.location.size(), std::string::npos);
	//si el archivo no existia y se ha creado devolver 201, si ya existia y ha sido modificado 200 o contenido vacio 204
	if ((open(path.c_str(), O_RDONLY)) == -1)
		client->setStatus("201 OK");
	client->_write_fd = open(path.c_str(), O_CREAT|O_WRONLY|O_NONBLOCK, 0666);
}

void createHeader(std::vector<Client>::iterator client)
{
	std::map<std::string, std::string> 	headers;
	
	std::string response = client->_sendInfo + " " + client->_status + "\r\n";
	if (client->_status == "405 Not Allowed")
		response = response + "Allow: " + client->_conf.method + "\r\n";
	response = response + "Sever: webserv/1.0.0\r\n";
	response = response + "Date: " + get_date() + "\r\n";  
	response = response + "Last-Modified: " + getLastModified(client->_path) + "\r\n"; //date de archivo requested by client
	response = response + "Content-Type: " + getDataType(client->_rFile) + "\r\n";
	client->setSendInfo(response); 	
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
		return ("application/octet-stream");
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
