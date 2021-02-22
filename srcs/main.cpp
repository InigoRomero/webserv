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
    {
		if(!(it->start(&rSet, &wSet, &readSet, &writeSet)))
            perror("start");
		FD_SET(it->_sockfd, &rSet);
    }
    std::cout << "Server waiting for connections..." << std::endl;
    for(;;)
    {
        readSet = rSet; //reset fds
		writeSet = wSet;
        select(max_fd(servers) + 1, &readSet, &writeSet, NULL, &timeout);
        for (std::vector<Server>::iterator it = servers.begin(); it != servers.end(); it++)
        {
            if (FD_ISSET((*it)._sockfd, &readSet))
                (*it).acceptNewClient(&readSet, &writeSet);   
            for (std::vector<Client>::iterator it2 = it->_clients.begin(); it2 != it->_clients.end(); /*it2++*/)
            {
                if (FD_ISSET(it2->_fd, &readSet))
                {
                    it->readRequest(it2);
                    FD_CLR(it2->_fd, &readSet);
                    it->proccessRequest(it2);
                }
                if (FD_ISSET(it2->_fd, &writeSet))
                {   
                    std::cout << "HOLA" << std::endl;
                    it->writeResponse(it2);
                    FD_CLR(it2->_fd, &writeSet);
                   // FD_CLR(it2->_fd, it2->_rSet);
                   // FD_CLR(it2->_fd, &readSet);
                    close(it2->_fd);
                }
                if (it2->_read_fd != -1)
                {
                    it2->readFd();
                    FD_CLR(it2->_fd, &readSet);
                    FD_SET(it2->_fd, &writeSet);
                    close(it2->_read_fd);
                }
                //check timeout to close connection
                if (it2->_lastDate.size() != 0 && compareTime(it2->_lastDate) >= 10)
                {
			        it->_clients.erase(it2);
                    std::cout << "Bye client" << std::endl;
                }
                else
                    it2++;
            }
        }
    }
    return (1);
}

int main(int argc, char **av)
{
	if (argc != 2) {
        std::cerr << "Usage: ./webserv config-file" << std::endl;
    	return (EXIT_FAILURE);
    }
    std::vector<Server> servers;
    Conf conf = Conf(av[1]);
    try {
        conf.ReadFile();
		conf.checkFile();
        conf.fillServer();
    }
    catch(std::exception const &e) {
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
        for (std::vector<struct location>::iterator it2 = (*it)._methods.begin(); it2 != (*it)._methods.end(); it2++)
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

/*      CERRAR CONEXION SI LLEVA X TIEMPO SIN HACER PETICIONES
case Client::STANDBY:
			if (getTimeDiff(client->last_date) >= TIMEOUT)
				client->status = Client::DONE;
			break ;
		case Client::DONE:
			delete client;
			_clients.erase(it);
			g_logger.log("[" + std::to_string(_port) + "] " + "connected clients: " + std::to_string(_clients.size()), LOW);
			return (0);
            
            */