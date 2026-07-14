#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int server_fd, new_socket; //file descriptor
    struct sockaddr_in address = {0};
    socklen_t addrlen = sizeof(address);
    char buffer[BUFFER_SIZE];

    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
        perror("socket cannot be created");
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = INADDR_ANY;

    if((bind(server_fd, (struct sockaddr*)&address, sizeof(address))) < 0) {
        perror("BIND ERROR");
        exit(EXIT_FAILURE);
    }

    if(listen(server_fd, 3) < 0) {
        perror("LISTEN ERROR");
        exit(EXIT_FAILURE);
    }

    printf("Server is listening at port %d\n", PORT);

    if ((new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen)) < 0) {
        perror("ACCEPT ERROR");
        exit(EXIT_FAILURE);
    }
    printf("Connection is succesfull\n");

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        
        int valread = recv(new_socket, buffer, BUFFER_SIZE, 0);

        if(valread < 0) { 
        perror("ERROR RECV ON SERVER SIDE");
        exit(EXIT_FAILURE);
        }

        if(valread == 0) {
            printf("Client disconnected.\n");
            break;
        }

        buffer[valread] = '\0';
        
        printf("Received message from client: %s\n", buffer);
        
        if(send(new_socket, buffer, BUFFER_SIZE, 0) < 0) {
            perror("ERROR SEND SERVER");
            exit(EXIT_FAILURE);
        }
    }

    close(new_socket);
    close(server_fd);

    return 0;
}