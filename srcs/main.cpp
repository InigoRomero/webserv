#include "server.hpp"
#include "conf.hpp"

int init(std::vector<Server> servers)
{
    fd_set					readSet;
	fd_set					writeSet;
	fd_set					rSet;
	fd_set					wSet;
    struct timeval			timeout;
    Client					*client;

	signal(SIGINT, exit);
	FD_ZERO(&rSet);
	FD_ZERO(&wSet);
	FD_ZERO(&readSet);
	FD_ZERO(&writeSet);
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	for (std::vector<Server>::iterator it(servers.begin()); it != servers.end(); ++it)
		if(!(it->start(&readSet, &writeSet, &rSet, &wSet)))
            perror("start");
    std::cout << "Server waiting for connections..." << std::endl;
    for(;;)
    {
        readSet = rSet; //reset fds
		writeSet = wSet;
        int maxFD = max_fd(servers);
        select(maxFD + 1, &readSet, &writeSet, NULL, &timeout);
       // std::cout << "Jaime"<< std::endl;
        for (std::vector<Server>::iterator it = servers.begin(); it != servers.end(); ++it)
        {
            if (FD_ISSET((*it)._sockfd, &readSet))
            {
                if (getOpenFd(servers) > maxFD)
					(*it).refuseConnection();
				else
                    (*it).acceptNewClient();
            }
            for (std::vector<Client*>::iterator it2 = it->_clients.begin(); it2 != it->_clients.end(); ++it2)
            {
                client = *it2;
                //std::cout << "Server FD: " << it->_sockfd << std::endl;
                //std::cout << "Cliente FD: " << client->_fd << std::endl;
                if (FD_ISSET(client->_fd, &readSet))                 
                {
                    if (!it->readRequest(it2))
                        break ;
                    if ((client->_lastDate.size() != 0 && compareTime(client->_lastDate) >= 10))
                    {
                        memset( client->_request->_rBuf, '\0', sizeof(char)*BUFFER_SIZE );
                        //free(client->_request->_rBuf);
                        close(client->_fd);
                        FD_CLR(client->_fd, client->_rSet);
                        FD_CLR(client->_fd, client->_wSet);
                        //it2 = it->_clients.erase(client);
                        std::cout << "Bye client" << std::endl;
                        break ;
                    }
                }
                if (client->_write_fd != -1)
                {
                    client->writeFd();
                    client->_lastDate = get_date();
                }
                if (client->_read_fd != -1)
                {
                    client->readFd();
                    client->_lastDate = get_date();
                }
                if (FD_ISSET(client->_fd, &writeSet))
                {
                    if (!it->writeResponse(it2))
                    {
                        client->_lastDate = get_date();
                        break ;
                    }
                }
            }
        }
    }
    return (1);
}

int main(int argc, char **av)
{
    Conf conf;
	if (argc != 2) {
        std::cerr << "Usage: ./webserv config-file" << std::endl;
    	return (EXIT_FAILURE);
    }
    std::vector<Server> servers;
    try {
        conf.setPath(av[1]);
        conf.ReadFile();
    }
    catch(std::exception const &e) {
		std::cerr << "[+] " << e.what() << std::endl;
        return (EXIT_FAILURE);
	}
    servers = conf.getServer();
    init(servers);
}