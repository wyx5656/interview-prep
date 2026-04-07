#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

// 创建稀疏文件
void create_sparse_file(const char *filename, off_t size) {
    int fd;

    // 打开或创建文件
    fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
    if (fd < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // 使用 ftruncate 来扩展文件到指定大小
    if (ftruncate(fd, size) != 0) {
        perror("ftruncate");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // 关闭文件
    close(fd);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <filename> <size_in_MB>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *filename = argv[1];
    int size_mb = atoi(argv[2]);

    if (size_mb <= 0) {
        fprintf(stderr, "Invalid size. Size must be a positive integer.\n");
        exit(EXIT_FAILURE);
    }

    // 将大小转换为字节（MB * 1024 * 1024）
    off_t size = (off_t)size_mb * 1024 * 1024;

    create_sparse_file(filename, size);

    printf("Sparse file '%s' created with size %d MB.\n", filename, size_mb);

    return 0;
}
