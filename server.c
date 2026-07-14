#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8080
#define BUFFER_SIZE 1024

//Function works seperetaly for each client
void *client_handler(void *socket_desc) {
    int sock = *(int*)socket_desc;
    char buffer[BUFFER_SIZE];
    int valread;


    free(socket_desc);

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        valread = recv(sock, buffer, BUFFER_SIZE, 0);

        if (valread <= 0) {
            printf("A client disconnected. Socket: %d\n", sock);
            break;
        }

        printf("Socket %d sent: %s", sock, buffer);
        send(sock, buffer, strlen(buffer), 0);
    }

    close(sock);
    pthread_exit(NULL);
}

int main() {
    int server_fd, new_socket; //file descriptor
    struct sockaddr_in address = {0};
    int addrlen = sizeof(address);

    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
        perror("socket cannot be created");
        exit(EXIT_FAILURE);
    }

    // Socket options
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
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

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen)) < 0) {
        perror("ACCEPT ERROR");
        exit(EXIT_FAILURE);
        }
        printf("New connection is succesfull. Socket: &d\n", new_socket);

        int *new_sock_ptr = malloc(sizeof(int));
        *new_sock_ptr = new_socket;

        pthread_t sniffer_thread;
        // Create a new thread and start client_handler func
        if (pthread_create(&sniffer_thread, NULL, client_handler, (void*)new_sock_ptr) < 0) {
            perror("Thread olusturulamadi");
            free(new_sock_ptr);
            continue;
        }

    }

    close(server_fd);

    return 0;
}