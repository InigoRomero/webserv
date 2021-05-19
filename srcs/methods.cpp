#include "methods.hpp"

void responseGet(std::vector<Client*>::iterator it)
{
	//std::cout << "responseGet\n";
	Client		*client = *it;
	//std::string response = client->_request->_version;
	std::string		path;
	std::string ext;
	int ret = 0;
	size_t pos;

	if ((pos = client->_path.find_last_of(".")) != std::string::npos)
	{
		client->setRFile(client->_path.substr(pos, std::string::npos));
		ext = client->_path.substr(pos, std::string::npos);
	}
	if ((client->_conf.cgi != ""  && client->_conf.cgi == ext))
	{
		client->_request->execCGI(*client);
		client->_request->_cgi = true;
	}
	else
	{
		if ((ret =  open(client->_path.c_str(), O_RDONLY)) == -1)
		{
			client->_chunkDone = true;
			client->setStatus("404 Not Found");
			contentNegotiation(it);
			return ;
		}
		// if exits
		//client->setPath(client->_path.c_str());
		client->setReadFd(ret);
	}
}

void responsePost(std::vector<Client*>::iterator it)
{
	Client		*client = *it;
	size_t pos;
	std::string ext;
	std::string	path;
		
	if ((pos = client->_path.find_last_of(".")) != std::string::npos)
		ext = client->_path.substr(pos, std::string::npos);
	if ((client->_conf.cgi != ""  && client->_conf.cgi == ext))
	{
		client->_request->execCGI(*client);
		client->_request->_cgi = true;
	}
	else
	{
		if ((open(client->_path.c_str(), O_RDONLY)) == -1)
		{
			client->_write_fd = open(client->_path.c_str(), O_CREAT|O_WRONLY|O_NONBLOCK, 0666);
			client->setStatus("201 OK");
		}
		else
			client->_write_fd = open(client->_path.c_str(), O_APPEND|O_WRONLY|O_NONBLOCK, 0666);
		if (client->_write_fd == -1)
			client->setStatus("500 Internal Server Error");
		else
			client->_chuckBody = "File modified\n";
		//std::cout << "writefd:" << client->_write_fd << std::endl;
		//if (client->_request->_req.size() == 0)
		//client->_chunkDone = true;
	}
}

void responsePut(std::vector<Client*>::iterator it)
{
	std::string	path;
	Client		*client = *it;

	//std::cout << "responsePut\n";
	//si el archivo no existia y se ha creado devolver 201, si ya existia y ha sido modificado 200 o contenido vacio 204
	if ((open(client->_path.c_str(), O_RDONLY)) == -1)
	{
		client->_chuckBody = "File created\n";
		client->setStatus("201 OK");
	}
	client->_write_fd = open(client->_path.c_str(), O_CREAT|O_WRONLY|O_NONBLOCK, 0666);
	if (client->_write_fd == -1)
		client->setStatus("500 Internal Server Error");
	else
		client->_chuckBody = "File modified\n";
	//client->_chunkDone = true;
}

void	responseHead(std::vector<Client*>::iterator it)
{
	(void)it;
	return;
}

void	responseDelete(std::vector<Client*>::iterator it)
{
	Client		*client = *it;
	int	ret;
	std::ifstream file(client->_path);

	if (file.good())
	{
		ret = remove(client->_path.c_str());
		if (!ret)
		{
			client->setStatus("200 OK");
			client->_chuckBody = "File deleted\n";
		}
	}
	else
		client->setStatus("204 No Content");
	client->_chunkDone = true;
}

static std::string allow_header(std::string str)
{
	std::string aux;
	if(str.find("GET") != std::string::npos)
		aux += "GET, ";
	if(str.find("POST") != std::string::npos)
		aux += "POST, ";
	if(str.find("HEAD") != std::string::npos)
		aux += "HEAD, ";
	if(str.find("PUT") != std::string::npos)
		aux += "PUT, ";
	if(str.find("DELETE") != std::string::npos)
		aux += "DELETE, ";
	return aux.substr(0, aux.size() - 2);
}

static std::map<std::string, std::string> parseAcceptHeaders(std::string aux)
{
	std::map<std::string, std::string> acceptMap;
	std::string aux2;
	size_t pos;

	while (aux != "")
	{
		aux2 = aux;
		if ((pos = aux.find(",")) != std::string::npos)
		{
			aux2 = aux.substr(0, pos);
			aux = aux.substr(pos + 1);
			if ((pos = aux2.find(";")) != std::string::npos)
				acceptMap.insert(std::pair<std::string, std::string>(aux2.substr(aux2.find_last_of(" ") + 1, pos - 1), aux2.substr(aux2.find("=") + 1)));
			else
				acceptMap.insert(std::pair<std::string, std::string>(aux2, "1"));
		}
		else
		{
			if ((pos = aux.find(";")) != std::string::npos)
				acceptMap.insert(std::pair<std::string, std::string>(aux.substr(aux.find_last_of(" ") + 1, pos - 1), aux.substr(aux.find("=") + 1)));
			else
				acceptMap.insert(std::pair<std::string, std::string>(aux, "1"));
			break;
		}
	}
	return acceptMap;
}

void contentNegotiation(std::vector<Client*>::iterator it)
{
	Client		*client = *it;
	std::map<std::string, std::string> lenguageMap;
	std::map<std::string, std::string> charsetMap;
	std::string path;
	std::string ext;
	int fd = -1;

	//std::cout << "Negotiate" << std::endl;
	//std::cout << client->_path << std::endl;
	lenguageMap = parseAcceptHeaders(client->_request->_headers["Accept-Language"]);
	charsetMap = parseAcceptHeaders(client->_request->_headers["Accept-Charset"]);
	if (!lenguageMap.empty())
	{
		for (std::multimap<std::string, std::string>::reverse_iterator it(lenguageMap.rbegin()); it != lenguageMap.rend(); ++it)
		{
			if (fd != -1)
				break;
			if (!charsetMap.empty())
			{
				for (std::multimap<std::string, std::string>::reverse_iterator it2(charsetMap.rbegin()); it2 != charsetMap.rend(); ++it2)
				{
					ext = it->first + "." + it2->first;
					path = client->_path + "." + ext;
					fd = open(path.c_str(), O_RDONLY);
					if (fd != -1)
					{
						client->_request->_headers["Content-Language"] = it->first;
						break ;
					}
					ext = it2->first + "." + it->first;
					path = client->_path + "." + ext;
					fd = open(path.c_str(), O_RDONLY);
					if (fd != -1)
					{
						client->_request->_headers["Content-Language"] = it->first;
						break ;
					}
				}
			}
			else
			{
				ext = it->first;
				path = client->_path + "." + ext;
				fd = open(path.c_str(), O_RDONLY);
				if (fd != -1)
				{
					client->_request->_headers["Content-Language"] = it->first;
					break ;
				}
			}
		}
	}
	else if (!charsetMap.empty())
	{
		for (std::multimap<std::string, std::string>::reverse_iterator it2(charsetMap.rbegin()); it2 != charsetMap.rend(); ++it2)
		{
			ext = it2->first;
			path = client->_path + "." + ext;
			fd = open(path.c_str(), O_RDONLY);
			if (fd != -1)
				break ;
		}
	}
	if (fd != -1)
	{
		client->_path = path;
		client->_request->_headers["Content-Location"] = client->_request->_uri + "." + ext;
		if (client->_read_fd != -1)
			close(client->_read_fd);
		client->_read_fd = fd;
		client->setStatus("200 OK");
	}
}

void createHeader(std::vector<Client*>::iterator it)
{
	Client		*client = *it;
	std::map<std::string, std::string> 	headers;

	std::string response = client->_sendInfo + " " + client->_status + "\r\n";
	response += "Sever: webserv/1.0.0\r\n";
	response += "Date: " + get_date() + "\r\n";
	if (client->_status == "405 Not Allowed")
		response += "Allow: " + allow_header(client->_conf.method) + "\r\n";
	else if (client->_status == "401 Unauthorized")
		response += "WWW-Authenticate = Basic\r\n";
	else if (client->_status == "201 OK")
		response += "Location: " + client->_request->_uri + "\r\n";
	else if (client->_status == "200 OK" && (client->_request->_method == "GET" || client->_request->_method == "HEAD"))
		response += "Last-Modified: " + getLastModified(client->_path) + "\r\n";
	if (client->_request->_cgi)
		response += "Content-Type: text/html; charset=utf-8\r\n";
	else if ((client->_request->_method == "POST" || client->_request->_method == "PUT") &&
	client->_request->_headers["Content-Type"] != "" && !client->_error)
		response += "Content-Type: " + client->_request->_headers["Content-Type"] + "\r\n";
	else
	{
		if (client->_error == true)
			response += "Content-Type: " + getDataType(client->_errorPath.substr(client->_errorPath.find_last_of("."))) + "\r\n";
		else
			response += "Content-Type: " + getDataType(client->_path.substr(client->_path.find_last_of("."))) + "\r\n";
	}
	if (client->_request->_headers["Content-Language"] != "")
		response += "Content-Language: " + client->_request->_headers["Content-Language"] + "\r\n";
	if (client->_request->_headers["Content-Location"] != "")
		response += "Content-Location: " + client->_request->_headers["Content-Location"] + "\r\n";
	client->setSendInfo(response);
	response.clear();
	//client->setSendInfo(client->_sendInfo + "Content-Type: text/html\r\n");
}

std::string getDataType(std::string fileExt)
{
	//std::cout << "fileExt" << fileExt << std::endl;
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
