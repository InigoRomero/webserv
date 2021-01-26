#ifndef CONF_HPP
#define CONF_HPP

#define UNAVAILABLE_TIME  20

#include <string>
#include <iostream>
#include <vector>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h> 
#include <netinet/ip.h>
#include <fcntl.h>
#include <sys/select.h>
#include <unistd.h>
#include <signal.h>
#include "getnextline/get_next_line.hpp"

class Conf
{
    private:
        Conf();
        const char *_path;
        std::vector<std::string> _conf;

    protected:

    public:

        Conf(const char *path);
		~Conf();

        class ConfigFileException: public std::exception {
            virtual const char* what() const throw();
        };
        void ReadFile();
        
};


#endif