#include "server.hpp"
#include "conf.hpp"

int max_fd(std::vector<Server> servers)
{
    int max = 0;
    int fd;

    for (std::vector<Server>::iterator it(servers.begin()); it != servers.end(); ++it)
    {
        fd = it->_sockfd;
        if (fd > max)
            max = fd;
    }
    return (max);
}

int init(std::vector<Server> servers)
{
    struct sockaddr_in	    client_addr;
    int                     addrlen;
    fd_set					readSet, writeSet, rSet, wSet;
    struct timeval			timeout;

    signal(SIGINT, exit);
	FD_ZERO(&rSet);
	FD_ZERO(&wSet);
	FD_ZERO(&readSet);
	FD_ZERO(&writeSet);
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	for (std::vector<Server>::iterator it(servers.begin()); it != servers.end(); ++it)
    {
		if(!(it->start()))
            perror("start");
		FD_SET(it->_sockfd, &rSet);
    }
    bzero(&client_addr, sizeof(client_addr));
    printf("server: waiting for connections...\n");
    for(;;)
    {
        readSet = rSet; //reset fds
		writeSet = wSet;
        addrlen = sizeof client_addr;
        select(max_fd(servers) + 1, &readSet, &writeSet, NULL, &timeout);
        for (std::vector<Server>::iterator it = servers.begin(); it != servers.end(); it++)
        {
            if (FD_ISSET((*it)._sockfd, &readSet))
            {
             //   if (config.getOpenFd(g_servers) > MAX_FD)
			//		s->refuseConnection();
		    //	else
                (*it).acceptNewClient(&readSet, &writeSet);   
            }
            for (std::vector<Client>::iterator it2 = it->_clients.begin(); it2 != it->_clients.end(); it2++)
            {
                if (FD_ISSET(it2->_fd, it2->_rSet))
                {
                    it->readRequest(it2);
                    FD_CLR(it2->_fd, it2->_rSet);
                    it->proccessRequest(it2);
                    FD_SET(it2->_fd, it2->_wSet);
                }

                if (FD_ISSET(it2->_fd, it2->_wSet))
                {   
                    it->writeResponse(it2);
                    FD_CLR(it2->_fd, it2->_wSet);
                  //  FD_SET(it2->_fd, it2->_rSet);
                    //FD_SET(it2->_fd, &rSet);
                   // it2 = it->_clients.erase(it2);
                }
                if (it2->_read_fd != -1)
                {
                    it2->readFD();
                    close(it2->_fd);
                    FD_CLR(it2->_fd, it2->_wSet);
                    FD_CLR(it2->_fd, it2->_rSet);
                    it2->setReadFD(-1);
                }
                }
        }
    }
    return (1);
}

int main(int argc, char **av)
{
	if (argc != 2)
    {
        std::cerr << "Usage: ./webserv config-file" << std::endl;
    	return (EXIT_FAILURE);
    }
    std::vector<Server> servers;
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
    init(servers);
    //system("leaks a.out");
    //SHOW ALL SERVERS CONF
    /*int i = 0;
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
    }*/
}