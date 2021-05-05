#ifndef METHODS_HPP
#define METHODS_HPP

#include "client.hpp"
#include "utils.hpp"

class Client;
class Helper;

void            responseGet(std::vector<Client*>::iterator it); 
void            responsePut(std::vector<Client*>::iterator it); 
void            responsePost(std::vector<Client*>::iterator it);
void            responseHead(std::vector<Client*>::iterator it);
void            responseDelete(std::vector<Client*>::iterator it);
void            createHeader(std::vector<Client*>::iterator it); 
std::string     getLastModified(std::string path);
std::string     getDataType(std::string fileExt);

#endif