#ifndef METHODS_HPP
#define METHODS_HPP

#include "server.hpp"
class Server;

void            responseGet(std::vector<Client*>::iterator it); 
void            responsePut(std::vector<Client*>::iterator it); 
void            responsePost(std::vector<Client*>::iterator it); 
void            createHeader(std::vector<Client*>::iterator it); 
std::string     getLastModified(std::string path);
std::string     getDataType(std::string fileExt);

#endif