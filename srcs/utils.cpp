#include "utils.hpp"

int max_fd(std::vector<Server> &servers)
{
    int max = 0;
    int fd;

    for (std::vector<Server>::iterator it(servers.begin()); it != servers.end(); ++it)
    {
        fd = it->getMaxFd();
        if (fd > max)
            max = fd;
    }
    return (max);
}

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

int	getOpenFd(std::vector<Server> &servers)
{
	int		nb = 0;

	for (std::vector<Server>::iterator it(servers.begin()); it != servers.end(); ++it)
	{
		nb += 1;
		nb += it->getOpenFd();
	}
	return (nb);
}

const std::string base64_chars =
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";


inline bool is_base64(unsigned char c)
{
  return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string base64_decode(std::string const& encoded_string)
{
  int in_len = encoded_string.size();
  int i = 0;
  int j = 0;
  int in_ = 0;
  unsigned char char_array_4[4], char_array_3[3];
  std::string ret;

  while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
    char_array_4[i++] = encoded_string[in_]; in_++;
    if (i ==4) {
      for (i = 0; i <4; i++)
        char_array_4[i] = base64_chars.find(char_array_4[i]);

      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for (i = 0; (i < 3); i++)
        ret += char_array_3[i];
      i = 0;
    }
  }

  if (i) {
    for (j = i; j <4; j++)
      char_array_4[j] = 0;

    for (j = 0; j <4; j++)
      char_array_4[j] = base64_chars.find(char_array_4[j]);

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

    for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
  }

  return ret;
}
