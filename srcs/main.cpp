#include "server.hpp"
#include "conf.hpp"

int main(int argc, char **av)
{
	if (argc != 2)
    {
        std::cerr << "Usage: ./webserv config-file" << std::endl;
    	return (EXIT_FAILURE);
    }
  
    Server serv = Server("Loco", 8080);
    serv.start();
    serv.acceptNewClient();
}