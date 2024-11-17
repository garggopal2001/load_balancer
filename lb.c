/* LOAD BALANCER PROCESS */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>     // For time functions
#include <sys/poll.h> // For polling

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
    int lb_sockfd, s1_sockfd, s2_sockfd, client_sockfd, s1_load = 0, s2_load = 0;
    int client_len;
    struct sockaddr_in lb_addr, server_addr, client_addr;
    struct pollfd poll_fdset;

    char buffer[100];

    if (argc != 4) {
        fprintf(stderr, "Usage: %s <LB Port> <S1 Port> <S2 Port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Create socket for Load Balancer */
    if ((lb_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Cannot create socket");
        exit(EXIT_FAILURE);
    }

    int lb_port = atoi(argv[1]);
    int s1_port = atoi(argv[2]);
    int s2_port = atoi(argv[3]);

    /* Set up Load Balancer address */
    lb_addr.sin_family = AF_INET;
    lb_addr.sin_addr.s_addr = INADDR_ANY;
    lb_addr.sin_port = htons(lb_port);

    /* Bind the socket to the Load Balancer address */
    if (bind(lb_sockfd, (struct sockaddr *)&lb_addr, sizeof(lb_addr)) < 0) {
        perror("Unable to bind local address");
        exit(EXIT_FAILURE);
    }

    /* Listen for client connections */
    listen(lb_sockfd, 5);

    time_t curr_time, prev_time;
    int timeout = 5000; // Timeout for polling in milliseconds

    while (1) {
        /* Set up polling */
        poll_fdset.fd = lb_sockfd;
        poll_fdset.events = POLLIN;
        time(&prev_time);

        printf("Polling for client connections with timeout %d seconds...\n", timeout / 1000);

        if (poll(&poll_fdset, 1, timeout) <= 0) {
            /* Poll timed out, request loads from servers */

            /* Connect to Server S1 */
            if ((s1_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                perror("Unable to create socket for Server S1");
                exit(EXIT_FAILURE);
            }

            server_addr.sin_family = AF_INET;
            inet_aton("127.0.0.1", &server_addr.sin_addr);
            server_addr.sin_port = htons(s1_port);

            if (connect(s1_sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
                perror("Unable to connect to Server S1");
                exit(EXIT_FAILURE);
            }

            /* Send "Send Load" request to Server S1 */
            strcpy(buffer, "Send Load");
            send(s1_sockfd, buffer, strlen(buffer) + 1, 0);

            /* Receive load from Server S1 */
            memset(buffer, 0, sizeof(buffer));
            receive_data(s1_sockfd, buffer);
            s1_load = atoi(buffer);

            /* Close connection to Server S1 */
            close(s1_sockfd);

            /* Connect to Server S2 */
            if ((s2_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                perror("Unable to create socket for Server S2");
                exit(EXIT_FAILURE);
            }

            server_addr.sin_port = htons(s2_port);

            if (connect(s2_sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
                perror("Unable to connect to Server S2");
                exit(EXIT_FAILURE);
            }

            /* Send "Send Load" request to Server S2 */
            strcpy(buffer, "Send Load");
            send(s2_sockfd, buffer, strlen(buffer) + 1, 0);

            /* Receive load from Server S2 */
            memset(buffer, 0, sizeof(buffer));
            receive_data(s2_sockfd, buffer);
            s2_load = atoi(buffer);

            /* Close connection to Server S2 */
            close(s2_sockfd);

            /* Print received loads */
            printf("Load received from Server S1: %d\n", s1_load);
            printf("Load received from Server S2: %d\n\n", s2_load);

            /* Reset timeout */
            timeout = 5000;
        } else {
            /* Incoming client connection */
            client_len = sizeof(client_addr);
            client_sockfd = accept(lb_sockfd, (struct sockaddr *)&client_addr, &client_len);
            if (client_sockfd < 0) {
                perror("Accept error");
                exit(EXIT_FAILURE);
            }

            if (fork() == 0) {
                /* Child process handles the client request */
                close(lb_sockfd);

                int server_sockfd;
                int server_port;
                char *server_name;

                /* Decide which server to send the request to */
                if (s1_load <= s2_load) {
                    server_port = s1_port;
                    server_name = "Server S1";
                } else {
                    server_port = s2_port;
                    server_name = "Server S2";
                }

                /* Connect to the selected server */
                if ((server_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                    perror("Unable to create socket to server");
                    exit(EXIT_FAILURE);
                }

                server_addr.sin_family = AF_INET;
                inet_aton("127.0.0.1", &server_addr.sin_addr);
                server_addr.sin_port = htons(server_port);

                if (connect(server_sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
                    perror("Unable to connect to server");
                    exit(EXIT_FAILURE);
                }

                printf("\nForwarding client request to %s\n", server_name);

                /* Send "Send Time" request to the server */
                strcpy(buffer, "Send Time");
                send(server_sockfd, buffer, strlen(buffer) + 1, 0);

                /* Receive date and time from the server */
                memset(buffer, 0, sizeof(buffer));
                receive_data(server_sockfd, buffer);

                /* Close connection to the server */
                close(server_sockfd);

                /* Send date and time back to the client */
                send(client_sockfd, buffer, strlen(buffer) + 1, 0);

                /* Close client socket and exit child process */
                close(client_sockfd);
                exit(0);
            }
            /* Parent process continues */
            close(client_sockfd);

            /* Calculate remaining time for next polling */
            time(&curr_time);
            timeout -= difftime(curr_time, prev_time) * 1000; // Adjust timeout in milliseconds
        }
    }

    /* Close Load Balancer socket */
    close(lb_sockfd);
    return 0;
}