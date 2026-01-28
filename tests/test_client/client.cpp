#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8000
#define BUF_SIZE 1024

int main() {
    char buffer[BUF_SIZE];

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    sockaddr_in server_addr{
        .sin_family = AF_INET,
        .sin_port = htons(SERVER_PORT)
    };
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    if (connect(sockfd, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        return 1;
    }

    printf("Connected\n");

    while (fgets(buffer, BUF_SIZE, stdin)) {
        send(sockfd, buffer, strlen(buffer), 0);

        int n = recv(sockfd, buffer, BUF_SIZE - 1, 0);
        if (n <= 0) break;

        buffer[n] = '\0';
        printf("Received: %s", buffer);
    }

    close(sockfd);
    return 0;
}
