#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8080
#define BUFFER_SIZE 1024

// Listen messages only from server
void *receive_handler(void *socket_desc) {
    int sock = *(int*)socket_desc;
    char buffer[BUFFER_SIZE];
    int valread;

    free(socket_desc);

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        valread = recv(sock, buffer, BUFFER_SIZE, 0);

        if (valread <= 0) {
            printf("\nConnection lost.\n");
            exit(EXIT_SUCCESS);
        }

        printf("%s", buffer);
        fflush(stdout); 
    }
    return NULL;
}

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

    // Create a thread which listens all messages from Server bg
    pthread_t recv_thread;
    int *sock_ptr = malloc(sizeof(int));
    *sock_ptr = sock;
    
    if (pthread_create(&recv_thread, NULL, receive_handler, (void*)sock_ptr) < 0) {
        perror("Thread Listen couldnot be created");
        return -1;
    }
    pthread_detach(recv_thread);

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        
        // Get from client and send it
        if(fgets(buffer, BUFFER_SIZE, stdin) == NULL) break;

        if(send(sock, buffer, strlen(buffer), 0) < 0) {
            perror("ERROR SEND");
            exit(EXIT_FAILURE);
        }
    }
}