//sources: 
// A python UDP ping client that we referenced: https://gist.github.com/ccormier/8db98d28c8990d66ce0c4b7233aed28f
// Some other source code we referenced: https://cboard.cprogramming.com/networking-device-communication/41635-ping-program.html

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#include <netdb.h>

#define TIMEOUT 1 
#define PING_COUNT 10
#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <host> <port>\n", argv[0]);
        exit(1);
    }
    // a lot of this stuff was pretty similar to the first lab, which we referenced
    char *host = argv[1];
    int port = atoi(argv[2]);
    
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    char send_buffer[BUFFER_SIZE];
    char receive_buffer[BUFFER_SIZE];
    
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, host, &server_addr.sin_addr);
    
    struct timeval timeout = {TIMEOUT, 0};
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        perror("Error setting timeout for socket");
        close(sockfd);
        exit(1);
    }

    struct hostent *serverEnt = gethostbyname(host);
    if (serverEnt == NULL) {
        fprintf(stderr, "Error: no such host %s\n", host);
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    
    int transmitted = 0, received = 0;
    double min_rtt = 1e9, max_rtt = 0, sum_rtt = 0;
    
    struct timeval time_sent, time_received;
    for (int i = 1; i <= PING_COUNT; i++) {
        transmitted += 1;
        int currTime = gettimeofday(&time_sent, NULL);
        if (currTime < 0) {
            perror("Unable to get current time");
            break;
        }

        snprintf(send_buffer, sizeof(send_buffer), "PING %d %ld.%06ld", i, (long)time_sent.tv_sec, (long)time_sent.tv_usec);
        if (sendto(sockfd, send_buffer, strlen(send_buffer), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            perror("Unable to send message");
            sleep(1);
            continue;
        }
        
        socklen_t addr_len = sizeof(server_addr);
        ssize_t numBytes = recvfrom(sockfd, receive_buffer, sizeof(receive_buffer)-1, 0, (struct sockaddr *)&server_addr, &addr_len);
        if (numBytes < 0) {
            // timeout occurred, packet was lost
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                printf("Request timeout for seq#=%d\n", i);
            } else {
                perror("receive error");
            }
        } else {
            // can calculate RTT
            receive_buffer[numBytes] = '\0';
            if (gettimeofday(&time_received, NULL) < 0) {
                perror("Unable to get current time");
                break;
            }

            double rtt = (time_received.tv_sec - time_sent.tv_sec) * 1000.0 + (time_received.tv_usec - time_sent.tv_usec) / 1000.0;
            sum_rtt += rtt;
            min_rtt = fmin(min_rtt, rtt);
            max_rtt = fmax(max_rtt, rtt);
            received += 1;
            printf("PING received from %s: seq#=%d time=%.3f ms\n",
                inet_ntoa(server_addr.sin_addr), i, rtt);
        }

        sleep(1);
    }

    int packet_lost = transmitted - received;
    double loss_rate;
    if (transmitted == 0) {
        loss_rate = 0;
    } else {
        loss_rate = (double)packet_lost / transmitted * 100;
    }

    double rtt_avg = 0;
    if (received > 0) {
        rtt_avg = sum_rtt / received;
    }

    printf("--- %s ping statistics ---\n", inet_ntoa(server_addr.sin_addr));
    if (received > 0) {
        printf("%d packets transmitted, %d received, %.0f%% packet loss rtt min/avg/max = %.3f %.3f %.3f ms\n",
            transmitted, received, loss_rate, min_rtt, rtt_avg, max_rtt);
    } else {
        printf("%d packets transmitted, %d received, %.0f%% packet loss\n",
            transmitted, received, loss_rate);
    }

    close(sockfd);
    return 0;
}