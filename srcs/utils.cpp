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

int				compareTime(std::string start)
{
	struct tm		start_tm;
	struct tm		*now_tm;
	struct timeval	time;
	int				result;

	strptime(start.c_str(), "%a, %d %b %Y %T", &start_tm);
	gettimeofday(&time, NULL);
	now_tm = localtime(&time.tv_sec);
	result = (now_tm->tm_hour - start_tm.tm_hour) * 3600;
	result += (now_tm->tm_min - start_tm.tm_min) * 60;
	result += (now_tm->tm_sec - start_tm.tm_sec);
	return (result);
}

void initMethods(struct location *methods)
{
	methods->location.clear();
    methods->method.clear();
    methods->root.clear();
    methods->index.clear();
    methods->cgi_path .clear();
    methods->cgi.clear();
    methods->auth.clear();

    methods->location = "";
    methods->method = "";
    methods->root = "";
    methods->index = "";
    methods->cgi_path = "";
    methods->cgi = "";
    methods->max_body = -1;
    methods->auto_index = -1;
    methods->auth = "";
}