#include "server.hpp"
#include "conf.hpp"
#define MAXDATASIZE 1000

int newAcceptNewClient(std::vector<Server> servers)
{
    struct sockaddr_in	client_addr;
    int addrlen;
    //fd_set read_fds;
    //fd_set master;
    fd_set					readSet;
    fd_set					writeSet;
    fd_set					rSet;
    fd_set					wSet;
    struct timeval			timeout;

    signal(SIGINT, exit);
	FD_ZERO(&rSet);
	FD_ZERO(&wSet);
	FD_ZERO(&readSet);
	FD_ZERO(&writeSet);
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	for (std::vector<Server>::iterator it(servers.begin()); it != servers.end(); ++it)
		it->start();
    for (std::vector<Server>::iterator it(servers.begin()); it != servers.end(); ++it)
		FD_SET(it->_sockfd, &rSet);
    bzero(&client_addr, sizeof(client_addr));
    for(;;)
    {
        readSet = rSet;
		writeSet = wSet;
        addrlen = sizeof client_addr;
        int		max = 0;
	    int		fd;

        for (std::vector<Server>::iterator it(servers.begin()); it != servers.end(); ++it)
        {
            fd = it->_sockfd;
            if (fd > max)
                max = fd;
        }
        select(max + 1, &readSet, &writeSet, NULL, &timeout);
        for (std::vector<Server>::iterator it = servers.begin(); it != servers.end(); it++)
        {
            if (FD_ISSET((*it)._sockfd, &readSet)) 
                readSet = (*it).acceptNewClient(readSet);
        }
    }
    return (1);
}

int main(int argc, char **av)
{
    std::vector<Server> servers;
	if (argc != 2)
    {
        std::cerr << "Usage: ./webserv config-file" << std::endl;
    	return (EXIT_FAILURE);
    }
    Conf conf = Conf(av[1]);
    try
	{
        conf.ReadFile();
		conf.checkFile();
        conf.fillServer();
    }catch(std::exception const &e)
	{
		std::cerr << "[+] " << e.what() << std::endl;
	}
    servers = conf.getServer();
  //  for (std::vector<Server>::iterator it = servers.begin(); it != servers.end(); it++)
   //     (*it).start();
   int i = 0;
    for (std::vector<Server>::iterator it = servers.begin(); it != servers.end(); it++)
    { 
        i++;
        std::cout << "Server:" << i << "\n{ "<< std::endl;
        std::cout << "  Lister: " << (*it)._port << std::endl;
        std::cout << "  Error: " << (*it)._error << std::endl;
        std::cout << "  Name: " << (*it)._name << std::endl;
        std::cout << "  Host: " << (*it)._host << std::endl;
        std::cout << "  Methods:  \n{ "<< std::endl;
        for (std::vector<struct methods>::iterator it2 = (*it)._methods.begin(); it2 != (*it)._methods.end(); it2++)
        {
            std::cout << "  {\n   Method type: " << (*it2).name << std::endl;
            std::cout << "   root: " << (*it2).root << std::endl;
            std::cout << "   index: " << (*it2).index << std::endl;
            std::cout << "   cgi: " << (*it2).cgi << std::endl;
            std::cout << "   cgi_path: " << (*it2).cgi_path << std::endl;
            std::cout << "   max_body: " << (*it2).max_body << "\n  }"<< std::endl;
        }
        std::cout << " }\n";
        std::cout << "}\n";
    }
   newAcceptNewClient(servers);
   // servers.back().acceptNewClient();


    /* SHOW ALL SERVERS CONF
    int i = 0;
    for (std::vector<Server>::iterator it = servers.begin(); it != servers.end(); it++)
    { 
        i++;
        std::cout << "Server:" << i << "\n{ "<< std::endl;
        std::cout << "  Lister: " << (*it)._port << std::endl;
        std::cout << "  Error: " << (*it)._error << std::endl;
        std::cout << "  Name: " << (*it)._name << std::endl;
        std::cout << "  Host: " << (*it)._host << std::endl;
        std::cout << "  Methods:  \n{ "<< std::endl;
        for (std::vector<struct methods>::iterator it2 = (*it)._methods.begin(); it2 != (*it)._methods.end(); it2++)
        {
            std::cout << "  {\n   Method type: " << (*it2).name << std::endl;
            std::cout << "   root: " << (*it2).root << std::endl;
            std::cout << "   index: " << (*it2).index << std::endl;
            std::cout << "   cgi: " << (*it2).cgi << std::endl;
            std::cout << "   cgi_path: " << (*it2).cgi_path << std::endl;
            std::cout << "   max_body: " << (*it2).max_body << "\n  }"<< std::endl;
        }
        std::cout << " }\n";
        std::cout << "}\n";
    } */
    //system("leaks a.out");
}