#include "server.hpp"
#include "conf.hpp"

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
    //system("leaks a.out");
}