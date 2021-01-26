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

class Conf
{
    private:
        Conf();

    protected:
        //

    public:
        std::string _path;

        Conf(std::string path);
		~Conf();
};


#endif