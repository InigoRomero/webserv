#ifndef METHODS_HPP
#define METHODS_HPP

#include "server.hpp"
class Server;

void            responseGet(std::vector<Client>::iterator client, Server serv); 
void            createHeader(std::vector<Client>::iterator client, Server serv); 
std::string     getLastModified(std::string path);

#endif