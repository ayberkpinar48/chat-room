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
    int sock;
    struct sockaddr_in server_address = {0};
    socklen_t addrlen  = sizeof(server_address);
    char buffer[BUFFER_SIZE];
    char message[BUFFER_SIZE] = {0};

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

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        
        printf("Message: ");
        if(fgets(buffer, BUFFER_SIZE, stdin) == NULL) break;

        if(send(sock, buffer, BUFFER_SIZE, 0) < 0) {
            perror("ERROR SEND");
            exit(EXIT_FAILURE);
        }
        printf("Message is sent to the server\n");

        int valread = recv(sock, buffer, BUFFER_SIZE, 0);
        if (valread <= 0) {
            printf("Server baglantiyi kesti.\n");
            break;
        }

        printf("Server'dan gelen cevap (Echo): %s", buffer);
    }

    close(sock);
    return 0;
}