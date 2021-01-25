#include "server.hpp"

int main()
{
    Server serv = Server("Loco", 8080);
    serv.start();
    serv.acceptNewClient();
}