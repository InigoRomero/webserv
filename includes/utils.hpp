#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include "location.hpp"

std::string		get_date();
int				compareTime(std::string start);
void            initMethods(struct location *methods);

#endif