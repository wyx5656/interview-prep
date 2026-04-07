#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/time.h>
#include <errno.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080

typedef struct {
    int conn_id;
    int success;
    int error;
} test_result_t;

typedef struct {
    int start_id;
    int end_id;
    test_result_t *results;
} thread_args_t;

void *test_connection(void *arg) {
    thread_args_t *args = (thread_args_t *)arg;

    for (int i = args->start_id; i < args->end_id; i++) {
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            args->results[i].success = 0;
            args->results[i].error = errno;
            continue;
        }

        struct sockaddr_in serv_addr;
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(SERVER_PORT);
        inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr);

        int ret = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
        if (ret < 0) {
            args->results[i].success = 0;
            args->results[i].error = errno;
            close(sockfd);
            continue;
        }

        args->results[i].success = 1;
        args->results[i].error = 0;
        close(sockfd);
        // Small delay to avoid too rapid connections
        usleep(1000);
    }

    return NULL;
}

int run_test(int total_conns, int num_threads) {
    printf("\n=== Testing with %d total connections, %d threads ===\n", total_conns, num_threads);

    test_result_t *results = malloc(total_conns * sizeof(test_result_t));
    if (!results) {
        printf("malloc failed\n");
        return -1;
    }
    memset(results, 0, total_conns * sizeof(test_result_t));

    thread_args_t *thread_args = malloc(num_threads * sizeof(thread_args_t));
    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));

    if (!thread_args || !threads) {
        printf("malloc failed\n");
        free(results);
        return -1;
    }

    struct timeval start, end;
    gettimeofday(&start, NULL);

    int conns_per_thread = total_conns / num_threads;

    for (int t = 0; t < num_threads; t++) {
        thread_args[t].start_id = t * conns_per_thread;
        if (t == num_threads - 1) {
            thread_args[t].end_id = total_conns;
        } else {
            thread_args[t].end_id = (t + 1) * conns_per_thread;
        }
        thread_args[t].results = results;
        pthread_create(&threads[t], NULL, test_connection, &thread_args[t]);
    }

    for (int t = 0; t < num_threads; t++) {
        pthread_join(threads[t], NULL);
    }

    gettimeofday(&end, NULL);

    int success_count = 0;
    int emfile_count = 0;
    int other_errors = 0;
    for (int i = 0; i < total_conns; i++) {
        if (results[i].success) {
            success_count++;
        } else {
            if (results[i].error == EMFILE) {
                emfile_count++;
            } else {
                other_errors++;
            }
        }
    }

    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
    double qps = total_conns / elapsed;

    printf("Success: %d/%d (%.2f%%)\n", success_count, total_conns,
           100.0 * success_count / total_conns);
    if (emfile_count > 0) {
        printf("  - EMFILE (too many open files): %d\n", emfile_count);
    }
    if (other_errors > 0) {
        printf("  - Other errors: %d\n", other_errors);
    }
    printf("Time elapsed: %.2f seconds\n", elapsed);
    printf("Connections per second: %.2f\n", qps);

    free(results);
    free(thread_args);
    free(threads);

    return success_count;
}

int main() {
    printf("CloudiskServer Concurrency Benchmark\n");
    printf("Server: %s:%d\n", SERVER_IP, SERVER_PORT);

    // 测试不同并发级别，逐步增加
    run_test(10, 2);
    sleep(1);
    run_test(20, 4);
    sleep(1);
    run_test(50, 10);
    sleep(1);
    run_test(100, 20);
    sleep(1);
    run_test(200, 40);

    printf("\n=== Benchmark Complete ===\n");

    return 0;
}
