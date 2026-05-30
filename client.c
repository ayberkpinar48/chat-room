#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sock;
    struct sockaddr_in server_address = {0};
    socklen_t addrlen  = sizeof(server_address);
    char buffer[BUFFER_SIZE];

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("SOCKET ERROR");
        exit(EXIT_FAILURE);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);

    if(inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) < 0){
        perror("Unvalid address");
        exit(EXIT_FAILURE);
    }

    printf("Trying to establish a connection with server\n");
    if(connect(sock, (struct sockaddr*)&server_address, addrlen) < 0){
        perror("CONNECTION ERROR");
        exit(EXIT_FAILURE);
    }
    printf("Connection is succesfull\n");


    printf("Message: ");
    fgets(buffer, BUFFER_SIZE, stdin);

    if(send(sock, buffer, BUFFER_SIZE, 0) < 0){
        perror("ERROR SEND");
        exit(EXIT_FAILURE);
    }
    printf("Message is sent to server\n");

    close(sock);
    return 0;
}