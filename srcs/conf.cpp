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
    while ((ret = get_next_line(fd, &line)))
    {
        std::string line2(line);
        line2.erase(std::remove_if(line2.begin(), line2.end(), isspace), line2.end());
        if(line2[0] == '#' || line2.empty())
            continue;
        _conf.push_back(line2);
        free(line);
    }
    if (line)
    {
        _conf.push_back(line);
        free(line);
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
            while ((*it).find("{") == std::string::npos)
            {
                if ((found = (*it).find("listen")) != std::string::npos)
                    _servers.back().setPort(stoi((*it).substr(found + 6, std::string::npos)));
                if ((found = (*it).find("error")) != std::string::npos)
                    _servers.back().setError((*it).substr(found + 5, std::string::npos));
                if ((found = (*it).find("name")) != std::string::npos)
                    _servers.back().setName((*it).substr(found + 4, std::string::npos));
                if ((found = (*it).find("host")) != std::string::npos)
                    _servers.back().setHost((*it).substr(found + 4, std::string::npos));

                it++;
            }
            while ((*it).find("}") == std::string::npos && it != _conf.end())
            {
                if ((*it).find("{") != std::string::npos)
                {
                    struct methods methods;
                    while ((*it).find("}") == std::string::npos && it != _conf.end())
                    {
                        if ((*it).find("method") == 0)
                            methods.name = (*it).substr(6, std::string::npos);
                        if ((*it).find("name") == 0)
                            methods.name = (*it).substr(4, std::string::npos);
                        if ((*it).find("root") == 0)
                            methods.root = (*it).substr(4, std::string::npos);
                        if ((*it).find("root") == 0)
                            methods.root = (*it).substr(4, std::string::npos);
                        if ((*it).find("index") == 0)
                            methods.index = (*it).substr(5, std::string::npos);
                        if ((*it).find("cgi") == 0)
                            methods.cgi = (*it).substr(3, std::string::npos);
                        if ((*it).find("cgi_path") == 0)
                            methods.cgi_path = (*it).substr(8, std::string::npos);
                        if ((*it).find("max_body") == 0)
                            methods.max_body = (*it).substr(8, std::string::npos);
                        it++;
                    }
                    _servers.back().setMethods(methods);
                    if (it + 1 != _conf.end())
                        it++;
                }
            }
        }
    }
}

std::vector<Server> Conf::getServer() const { return (_servers); }