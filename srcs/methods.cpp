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
		if (client->_request->_req.size() == 0)
			client->_chunkDone = true;
	}
}

void responsePut(std::vector<Client*>::iterator it)
{
	std::string	path;
	Client		*client = *it;

	std::cout << "responsePut\n";
	//si el archivo no existia y se ha creado devolver 201, si ya existia y ha sido modificado 200 o contenido vacio 204
	if ((open(client->_path.c_str(), O_RDONLY)) == -1)
		client->setStatus("201 OK");
	client->_write_fd = open(client->_path.c_str(), O_CREAT|O_WRONLY|O_NONBLOCK, 0666);
	client->_chunkDone = true;
}

void	responseHead(std::vector<Client*>::iterator it)
{
	(void)it;
	return;
}

void	responseDelete(std::vector<Client*>::iterator it)
{
	Client		*client = *it;
	std::string		path;
	//int ret = 0;

	if (client->_conf.location.size() < client->_request->_uri.size())
	{
		if (client->_request->_uri.find(".") == std::string::npos)
			path =  client->_conf.root + "/"+ client->_request->_uri.substr(client->_conf.location.size(), std::string::npos) + "/" + client->_conf.index;
		else
			path =  client->_conf.root + client->_request->_uri.substr(client->_conf.location.size(), std::string::npos);
	}
	else
		path = client->_conf.root + "/"+ client->_conf.index;

	client->_chuckBody = "File deleted\n";
	return;
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

static const std::string base64_chars =
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";


static inline bool is_base64(unsigned char c) {
  return (isalnum(c) || (c == '+') || (c == '/'));
}

static std::string base64_decode(std::string const& encoded_string) {
  int in_len = encoded_string.size();
  int i = 0;
  int j = 0;
  int in_ = 0;
  unsigned char char_array_4[4], char_array_3[3];
  std::string ret;

  while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
    char_array_4[i++] = encoded_string[in_]; in_++;
    if (i ==4) {
      for (i = 0; i <4; i++)
        char_array_4[i] = base64_chars.find(char_array_4[i]);

      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for (i = 0; (i < 3); i++)
        ret += char_array_3[i];
      i = 0;
    }
  }

  if (i) {
    for (j = i; j <4; j++)
      char_array_4[j] = 0;

    for (j = 0; j <4; j++)
      char_array_4[j] = base64_chars.find(char_array_4[j]);

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

    for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
  }

  return ret;
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
	for (std::map<std::string, std::string>::iterator it(lenguageMap.begin()); it != lenguageMap.end(); ++it)
	{
		std::cout << it->first << std::endl;
		std::cout << it->second << std::endl;
	}
	std::cout << "\n\n";
	for (std::map<std::string, std::string>::iterator it(charsetMap.begin()); it != charsetMap.end(); ++it)
	{
		std::cout << it->first << std::endl;
		std::cout << it->second << std::endl;
	}
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
						std::cout << "hola\n";
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

	std::cout << "holi\n";
	if (client->_conf.auth != "")
	{
		client->setStatus("401 Unauthorized");
		if (client->_request->_headers["Authorization"] != "")
		{
			if (client->_conf.auth == base64_decode(client->_request->_headers["Authorization"]))
				client->setStatus("200 OK");
		}	
	}
	std::cout << "sendinfo:\n" << client->_sendInfo << std::endl;
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
