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
#define MAX_CLIENTS 100

int clients[MAX_CLIENTS];
int client_count = 0;

// Mutex to prevent Race Condition
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;


void broadcast_message(char *message, int sender_sock) {
    pthread_mutex_lock(&clients_mutex); // Listeyi kitle

    for (int i = 0; i < client_count; i++) {
        if (clients[i] != sender_sock) {
            if (send(clients[i], message, strlen(message), 0) < 0) {
                perror("BROADCAST ERROR");
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// Bir client ayrildiginda listeden silen fonksiyon
void remove_client(int sock) {
    pthread_mutex_lock(&clients_mutex);

    for (int i = 0; i < client_count; i++) {
        if (clients[i] == sock) {
            // Silinen elemanin yerine dizideki son elemani koyuyoruz
            clients[i] = clients[client_count - 1];
            client_count--;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

//Function works seperetaly for each client
void *client_handler(void *socket_desc) {
    int sock = *(int*)socket_desc;
    char buffer[BUFFER_SIZE];
    char message[BUFFER_SIZE + 50]; // "Soket X: [mesaj]" formati icin biraz daha buyuk bir alan
    int valread;

    free(socket_desc);

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        memset(message, 0, sizeof(message));

        valread = recv(sock, buffer, BUFFER_SIZE, 0);

        if (valread <= 0) {
            printf("Soket %d odadan ayrildi.\n", sock);
            remove_client(sock);
            
            sprintf(message, "--- Soket %d odadan ayrildi. ---\n", sock);
            broadcast_message(message, sock);
            break;
        }

        printf("[LOG] Soket %d: %s", sock, buffer);
        sprintf(message, "User_%d: %s", sock, buffer);
        broadcast_message(message, sock);
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
        continue;
        }
        printf("New connection is succesfull. Socket: %d\n", new_socket);

        // Add new connection to the list
            pthread_mutex_lock(&clients_mutex);
            if (client_count < MAX_CLIENTS) {
                clients[client_count] = new_socket;
                client_count++;
                printf("New client. Socket: %d. Total amount of clients: %d\n", new_socket, client_count);
                pthread_mutex_unlock(&clients_mutex);

                // Inform everyone
                char welcome_msg[100];
                sprintf(welcome_msg, "--- User_%d attended to the room! ---\n", new_socket);
                broadcast_message(welcome_msg, new_socket);

                // Create a thread and run
                int *new_sock_ptr = malloc(sizeof(int));
                *new_sock_ptr = new_socket;
                pthread_t sniffer_thread;
                if (pthread_create(&sniffer_thread, NULL, client_handler, (void*)new_sock_ptr) < 0) {
                    perror("Thread could not be created");
                    free(new_sock_ptr);
                } else {
                    pthread_detach(sniffer_thread);
                }
            } else {
            printf("Room is full! Socket %d is denied.\n", new_socket);
            close(new_socket);
            pthread_mutex_unlock(&clients_mutex);
        }
}    

    close(server_fd);

    return 0;
}