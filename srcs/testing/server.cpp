// Server side C program to demonstrate Socket programming
// all EXPLICATION ON = https://medium.com/from-the-scratch/http-server-what-do-you-need-to-know-to-build-a-simple-http-server-from-scratch-d1ef8945e4fa
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <fstream>
#include <streambuf>

#define PORT 80
int main()
{
    int server_fd, new_socket; long valread;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    
    //char hello[28] = "<h1>Hello from server</h1>\n";
    
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("In socket");
        exit(EXIT_FAILURE);
    }
    

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
    
    memset(address.sin_zero, '\0', sizeof address.sin_zero);
    
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("In bind");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 10) < 0)
    {
        perror("In listen");
        exit(EXIT_FAILURE);
    }

  /*  FILE * pFile;
    pFile = fopen ("../www/index.html","w");
    if (pFile!=NULL)
    {
        fputs ("fopen example",pFile);
        fclose (pFile);
    }*/
    std::ifstream t("www/index.html");
    std::string str((std::istreambuf_iterator<char>(t)),
                 std::istreambuf_iterator<char>());
    char cstr[str.size() + 1];
    strcpy(cstr, str.c_str());
    while(1)
    {
        printf("\n+++++++ Waiting for new connection ++++++++\n\n");
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
        {
            perror("In accept");
            exit(EXIT_FAILURE);
        }
        
        char buffer[30000] = {0};
        valread = read( new_socket , buffer, 30000);
        printf("%s\n",buffer );
       // send(new_socket, pFile, pFile.tellg(), 0); //escribir con send
       // write(new_socket , hello , strlen(hello));
        write(new_socket , cstr, strlen(cstr));
        printf("------------------Hello message sent-------------------\n");
        close(new_socket);
    }
    return 0;
}