#include "conf.hpp"

Conf::Conf():
    _path()
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
        _conf.push_back(line);
        free(line);
        std::cout << _conf.back() << std::endl;
    }
    /*for (std::vector<std::string>::iterator it = _conf.begin(); it != _conf.end(); it++)
    {
        std::cout << *it << std::endl;
    }*/
    if (line)
        free(line);
    if (ret < 0)
        throw ConfigFileException();
}