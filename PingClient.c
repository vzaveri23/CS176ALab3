// #include <stdio.h>
// #include <stdlib.h>
// #include <math.h>
// #include <string.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
// #include <unistd.h>
// #include <time.h>

// #define TIMEOUT 1 
// #define PING_COUNT 10

// int main(int argc, char *argv[]) {
//     if (argc != 3) {
//         fprintf(stderr, "Usage: %s <host> <port>\n", argv[0]);
//         exit(1);
//     }
    
//     char *host = argv[1];
//     int port = atoi(argv[2]);
    
//     int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
//     if (sockfd < 0) {
//         perror("Socket creation failed");
//         exit(1);
//     }
    
//     struct sockaddr_in server_addr;
//     memset(&server_addr, 0, sizeof(server_addr));
//     server_addr.sin_family = AF_INET;
//     server_addr.sin_port = htons(port);
//     inet_pton(AF_INET, host, &server_addr.sin_addr);
    
//     struct timeval timeout = {TIMEOUT, 0};
//     setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    
//     int transmitted = 0, received = 0;
//     double rtts[PING_COUNT];
//     rtts[0] = 0.0;
//     double min_rtt = 1e6, max_rtt = 0, sum_rtt = 0;
    
//     for (int i = 1; i <= PING_COUNT; i++) {
//         char message[64];
//         struct timespec start, end;
//         snprintf(message, sizeof(message), "PING %d %ld", i, time(NULL));
//         transmitted++;
        
//         clock_gettime(CLOCK_MONOTONIC, &start);
//         sendto(sockfd, message, strlen(message), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
        
//         char buffer[64];
//         socklen_t addr_len = sizeof(server_addr);
//         if (recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&server_addr, &addr_len) >= 0) {
//             clock_gettime(CLOCK_MONOTONIC, &end);
//             double rtt = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_nsec - start.tv_nsec) / 1.0e6;
//             rtts[received++] = rtt;
//             sum_rtt += rtt;
//             if (rtt < min_rtt) min_rtt = rtt;
//             if (rtt > max_rtt) max_rtt = rtt;
//             printf("PING received from %s: seq#=%d time=%.2f ms\n", host, i, rtt);
//         } else {
//             printf("Request timeout for seq#=%d\n", i);
//         }
//         sleep(1);
//     }
    
//     double avg_rtt = received ? sum_rtt / received : 0;
//     int loss = ((PING_COUNT - received) * 100) / PING_COUNT;
//     printf("--- %s ping statistics ---\n", host);
//     printf("%d packets transmitted, %d received, %d%% packet loss\n", transmitted, received, loss);
//     if (received) {
//         // printf("rtt min/avg/max = %.3f %.3f %.3f ms\n", min_rtt, avg_rtt, max_rtt);
//         printf("rtt min/avg/max = %.3f %.3f %.3f ms\n", 
//         round(min_rtt * 1000) / 1000.0,
//         round(avg_rtt * 1000) / 1000.0,
//         round(max_rtt * 1000) / 1000.0);
//     }
    
//     close(sockfd);
//     return 0;
// }
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

#define TIMEOUT 1 
#define PING_COUNT 10

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <host> <port>\n", argv[0]);
        exit(1);
    }
    
    char *host = argv[1];
    int port = atoi(argv[2]);
    
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(1);
    }
    
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, host, &server_addr.sin_addr);
    
    struct timeval timeout = {TIMEOUT, 0};
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    
    int transmitted = 0, received = 0;
    double min_rtt = 1e6, max_rtt = 0, sum_rtt = 0;
    
    for (int i = 1; i <= PING_COUNT; i++) {
        char message[64];
        struct timespec start, end;
        snprintf(message, sizeof(message), "PING %d %ld", i, time(NULL));
        transmitted++;
        
        clock_gettime(CLOCK_MONOTONIC, &start);
        sendto(sockfd, message, strlen(message), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
        
        char buffer[64];
        socklen_t addr_len = sizeof(server_addr);
        if (recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&server_addr, &addr_len) >= 0) {
            clock_gettime(CLOCK_MONOTONIC, &end);
            double rtt = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_nsec - start.tv_nsec) / 1.0e6;
            received++;
            sum_rtt += rtt;
            if (rtt < min_rtt) min_rtt = rtt;
            if (rtt > max_rtt) max_rtt = rtt;
            printf("PING received from %s: seq#=%d time=%.2f ms\n", host, i, rtt);
        } else {
            printf("Request timeout for seq#=%d\n", i);
        }
        sleep(1);
    }
    
    double avg_rtt = received ? sum_rtt / received : 0;
    int loss = ((PING_COUNT - received) * 100) / PING_COUNT;
    printf("--- %s ping statistics ---\n", host);
    printf("%d packets transmitted, %d received, %d%% packet loss\n", transmitted, received, loss);
    if (received) {
        printf("rtt min/avg/max = %.3f %.3f %.3f ms\n", 
            round(min_rtt * 1000) / 1000.0,
            round(avg_rtt * 1000) / 1000.0,
            round(max_rtt * 1000) / 1000.0);
    }
    
    close(sockfd);
    return 0;
}
