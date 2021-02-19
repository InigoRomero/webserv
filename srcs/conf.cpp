#include "conf.hpp"

Conf::Conf():
    _path(), _conf(), _servers()
{}

Conf::Conf(const char *path):
    _path(path)
{}

Conf::~Conf()
{}

const char* Conf::ConfigFileException::what() const throw()
{
    return "Invalid config file";
}

void Conf::ReadFile()
{
    int fd;
    int ret;
    char *line = NULL;

    if ((fd = open(_path, O_RDONLY)) == -1)
        throw ConfigFileException();
    while ((ret = get_next_line(fd, &line)) >= 0)
    {
        std::string line2(line);
        line2.erase(std::remove_if(line2.begin(), line2.end(), isspace), line2.end());
        if(!(line2[0] == '#' || line2.empty()))
            _conf.push_back(line2);
        if (line)
            free(line);
        if (ret == 0)
            break ;
    }
    if (ret < 0)
        throw ConfigFileException();
}

void Conf::checkFile()
{
    int sum = 0;
    for (std::vector<std::string>::iterator it = _conf.begin(); it != _conf.end(); it++)
    {
        if ((*it).find("{") != std::string::npos)
            sum++;
        else if ((*it).find("}") != std::string::npos)
            sum--;
        if (sum >= 3)
            throw ConfigFileException();
    }
    if (sum != 0)
        throw ConfigFileException();
}

void Conf::fillServer()
{
    for (std::vector<std::string>::iterator it = _conf.begin(); it != _conf.end(); it++)
    {
      //  std::cout << *it << std::endl;
        size_t found;
        if ((*it).find("server{") != std::string::npos)
        {
            it++;
            _servers.push_back(Server());
            while ((*it).find("{") == std::string::npos && (*it).find("}") == std::string::npos)
            {
                if ((found = (*it).find("listen")) != std::string::npos)
                    _servers.back().setPort(stoi((*it).substr(found + 6, std::string::npos)));
                else if ((found = (*it).find("error")) != std::string::npos)
                    _servers.back().setError((*it).substr(found + 5, std::string::npos));
                else if ((found = (*it).find("name")) != std::string::npos)
                    _servers.back().setName((*it).substr(found + 4, std::string::npos));
                else if ((found = (*it).find("host")) != std::string::npos)
                    _servers.back().setHost((*it).substr(found + 4, std::string::npos));
                else
                    throw ConfigFileException();
                it++;
            }
            _servers.back().setConf(_path);
            while ((*it).find("}") == std::string::npos)
            {
                if ((*it).find("location") != std::string::npos) //falta coger texto location
                {
                    struct location p;
                    initMethods(&p);
                    p.location = (*it).substr(8, std::string::npos);
                    p.location = p.location.substr(0, p.location.length() - 1);
                    it++;
                    while ((*it).find("}") == std::string::npos) //compare en vez de find por si  "}" no esta en una linea a parte??
                    {
                        if ((*it).find("method") == 0)
                            p.method = (*it).substr(6, std::string::npos);
                        else if ((*it).find("root") == 0)
                            p.root = (*it).substr(4, std::string::npos);
                        else if ((*it).find("index") == 0)
                            p.index = (*it).substr(5, std::string::npos);
                        else if ((*it).find("cgi_path") == 0)
                            p.cgi_path = (*it).substr(8, std::string::npos);
                        else if ((*it).find("cgi") == 0)
                            p.cgi = (*it).substr(3, std::string::npos);
                        else if ((*it).find("max_body") == 0)
                            p.max_body = stoi((*it).substr(8, std::string::npos));
                        else if ((*it).find("auto_index") == 0)
                            p.auto_index = stoi((*it).substr(10, std::string::npos));
                        else if ((*it).find("auth") == 0)
                            p.auth = (*it).substr(4, std::string::npos);
                        else
                           throw ConfigFileException();
                        it++;
                    }
                    _servers.back().setMethods(p);
                    it++;
                }
            }
        }
        else
            throw ConfigFileException();
    }
}

std::vector<Server> Conf::getServer() const { return (_servers); }