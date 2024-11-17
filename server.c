/* SERVER PROCESS (S1 and S2) */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

/* Function to receive data in chunks */
void receive_data(int sockfd, char *buffer) {
    int len, index = 0, flag = 0;
    char temp_buffer[50];
    while (1) {
        len = recv(sockfd, temp_buffer, sizeof(temp_buffer), 0);
        if (len <= 0)
            break;
        for (int i = 0; i < len; i++) {
            buffer[index++] = temp_buffer[i];
            if (temp_buffer[i] == '\0') {
                flag = 1;
                break;
            }
        }
        if (flag == 1)
            break;
    }
}

int main(int argc, char *argv[]) {
    int server_sockfd, client_sockfd;
    int client_len;
    struct sockaddr_in server_addr, client_addr;

    char buffer[100];

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <Server Port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Seed the random number generator */
    srand(time(0));

    /* Create socket */
    if ((server_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Cannot create socket");
        exit(EXIT_FAILURE);
    }

    int server_port = atoi(argv[1]);

    /* Set up server address */
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(server_port);

    /* Bind the socket to the server address */
    if (bind(server_sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Unable to bind local address");
        exit(EXIT_FAILURE);
    }

    /* Listen for connections */
    listen(server_sockfd, 5);

    while (1) {
        client_len = sizeof(client_addr);
        client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_addr, &client_len);

        if (client_sockfd < 0) {
            perror("Accept error");
            exit(EXIT_FAILURE);
        }

        /* Receive request from Load Balancer */
        memset(buffer, 0, sizeof(buffer));
        receive_data(client_sockfd, buffer);

        if (strcmp(buffer, "Send Time") == 0) {
            /* Handle "Send Time" request */
            time_t current_time = time(NULL);
            char *time_str = ctime(&current_time);
            time_str[strlen(time_str) - 1] = '\0'; // Remove newline character

            strcpy(buffer, time_str);

            /* Send date and time back to Load Balancer */
            send(client_sockfd, buffer, strlen(buffer) + 1, 0);
        } else if (strcmp(buffer, "Send Load") == 0) {
            /* Handle "Send Load" request */
            int load = rand() % 100 + 1; // Generate random load between 1 and 100
            printf("Load sent: %d\n", load); // Print the load sent

            sprintf(buffer, "%d", load);

            /* Send load back to Load Balancer */
            send(client_sockfd, buffer, strlen(buffer) + 1, 0);
        }

        /* Close client socket */
        close(client_sockfd);
    }

    /* Close server socket */
    close(server_sockfd);
    return 0;
}