#include "utils.hpp"

std::string		get_date()
{
	struct timeval	time;
	struct tm		*tm;
	char			buf[1000];
	int				ret;

	gettimeofday(&time, NULL);
	tm = localtime(&time.tv_sec);
	ret = strftime(buf, sizeof(buf), "%a, %d %b %Y %T %Z", tm);
	buf[ret] = '\0';
	return (buf);
}