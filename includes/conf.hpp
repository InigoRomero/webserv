#ifndef CONF_HPP
#define CONF_HPP

#define UNAVAILABLE_TIME  20

#include "get_next_line.hpp"
#include "server.hpp"

class Conf
{
    private:
        Conf();
        const char                  *_path;
        std::vector<std::string>    _conf;
        std::vector<Server>         _servers;
    protected:

    public:

        Conf(const char *path);
		~Conf();

        class ConfigFileException: public std::exception {
            virtual const char* what() const throw();
        };
        void                ReadFile(); //get in vector of strings all the data
        void                fillServer(); //get the info from the conf vector to Server vector object
        void                checkFile();
        void                initMethods(struct methods *methods);
        std::vector<Server> getServer() const; 
        //std::vector<Server> GetInfo();
        
};


#endif