#include <sys/socket.h> // For socket functions
#include <netinet/in.h> // For sockaddr_in
#include <cstdlib> // For exit() and EXIT_FAILURE
#include <iostream> // For cout
#include <unistd.h> // For read
#include <string>

using namespace std;

int main() {
  // Create a socket (IPv4, TCP)
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1) {
    cout << "Failed to create socket. errno: " << errno << std::endl;
    exit(EXIT_FAILURE);
  }

  // Listen to port 9999 on any address
  sockaddr_in sockaddr;
  sockaddr.sin_family = AF_INET; //define que la conexion es IPv4, AF_INET6 - IPv6 Internet protocols
  sockaddr.sin_addr.s_addr = INADDR_ANY; //Estructura a la dirección IP
  sockaddr.sin_port = htons(9999); // Puerto para la conexión
                                    // htons is necessary to convert a number to
                                   // network byte order
  if (bind(sockfd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0) { //bind: to assign an IP address and port to the socket. (dar un nombre al socket, identificarlo.)
    cout << "Failed to bind to port 9999. errno: " << errno << std::endl;
    exit(EXIT_FAILURE);
  }

  // Start listening. Hold at most 10 connections in the queue
  if (listen(sockfd, 10) < 0) {
    cout << "Failed to listen on socket. errno: " << errno << std::endl;
    exit(EXIT_FAILURE);
  }

  // Grab a connection from the queue
  size_t addrlen = sizeof(sockaddr);
  int connection = accept(sockfd, (struct sockaddr*)&sockaddr, (socklen_t*)&addrlen); //accept extracts an element from a queue of connections
  if (connection < 0) {
    cout << "Failed to grab connection. errno: " << errno << std::endl;
    exit(EXIT_FAILURE);
  }

  // Read from the connection
    signal(SIGINT, exit);
    char buffer[101];
    read(connection, buffer, 100);
    buffer[100] = '\0';
    cout << "The message was: " << buffer << " Bye";

    // Send a message to the connection
    string response = "Good talking to you\n"; 
    send(connection, response.c_str(), response.size(), 0);
    // Close the connections
    std::cout << "Closing Server..." << std::endl;
    close(connection);
    close(sockfd);
}