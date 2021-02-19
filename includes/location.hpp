#ifndef LOCATION_HPP
#define LOCATION_HPP

struct location {
    std::string location;
    std::string method;
    std::string root;
    std::string index;
    std::string cgi;
    std::string cgi_path;
    int         max_body;
    int         auto_index;
    std::string auth;
};

#endif