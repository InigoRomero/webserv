#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include "location.hpp"
#include "server.hpp"

class Server;

std::string		get_date();
int				compareTime(std::string start);
void            initMethods(struct location *methods);
int				getOpenFd(std::vector<Server> &servers);
int             max_fd(std::vector<Server> &servers);
inline bool     is_base64(unsigned char c);
std::string     base64_decode(std::string const& encoded_string);

#endif