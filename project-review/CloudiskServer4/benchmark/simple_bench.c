#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    int n = atoi(argv[1]);
    if (n <= 0) n = 100;

    printf("Benchmark: %d sequential connections to 127.0.0.1:8080\n", n);

    struct timeval start, end;
    gettimeofday(&start, NULL);

    int success = 0;
    int failed = 0;

    for (int i = 0; i < n; i++) {
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            failed++;
            continue;
        }

        struct sockaddr_in serv_addr;
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(8080);
        inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

        int ret = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
        if (ret < 0) {
            failed++;
        } else {
            success++;
        }

        close(sockfd);
    }

    gettimeofday(&end, NULL);

    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;

    printf("Results:\n");
    printf("  Success: %d\n  Failed: %d\n  Success rate: %.2f%%\n",
           success, failed, 100.0 * success / (success + failed));
    printf("  Time: %.2f seconds\n  QPS: %.2f\n", elapsed, n / elapsed);

    return 0;
}
