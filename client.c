/* CLIENT PROCESS */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* Function to receive data in chunks */
void receive_data(int sockfd, char *buffer) {
    int len, index = 0, flag = 0;
    char temp_buffer[50];
    while (1) {
        len = recv(sockfd, temp_buffer, 50, 0);
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
    int sockfd;
    struct sockaddr_in server_addr;

    char buffer[100];

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <Load Balancer IP> <Port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Create a socket */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Unable to create socket");
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[2]);

    /* Set up the server address structure */
    server_addr.sin_family = AF_INET;
    inet_aton(argv[1], &server_addr.sin_addr); // Use server IP from command line
    server_addr.sin_port = htons(port);

    /* Connect to the Load Balancer */
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Unable to connect to load balancer");
        exit(EXIT_FAILURE);
    }

    /* Receive date and time from Load Balancer */
    memset(buffer, 0, sizeof(buffer));
    receive_data(sockfd, buffer);

    /* Print the received date and time */
    printf("System Date and Time: %s\n", buffer);

    /* Close the socket */
    close(sockfd);
    return 0;
}