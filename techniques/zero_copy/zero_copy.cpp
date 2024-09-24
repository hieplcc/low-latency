#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <cstdio>
#include <cerrno>
#include <chrono>
#include <iostream>

void zero_copy(const char* src, const char* des) {
    auto start = std::chrono::high_resolution_clock::now();

    int src_fd = open(src, O_RDONLY);
    if (src_fd == -1) {
        perror("Failed to open source file");
        exit(EXIT_FAILURE);
    }

    struct stat st;
    fstat(src_fd, &st);

    int des_fd = open(des, O_WRONLY | O_CREAT, st.st_mode);
    if (des_fd == -1) {
        perror("Failed to open destinationi file");
        close(src_fd);
        exit(EXIT_FAILURE);
    }

    off_t offset = 0;
    int rc = sendfile(des_fd, src_fd, &offset, st.st_size);
    if (rc != st.st_size) {
        perror("Failed to sendfile");
        close(src_fd);
        close(des_fd);
        exit(1);
    }

    close(src_fd);
    close(des_fd);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "zero-copy takes: " << duration << " (ms)" << std::endl;
}

void copy(const char* src, const char* des) {
    auto start = std::chrono::high_resolution_clock::now();
    
    int src_fd = open(src, O_RDONLY);
    if (src_fd == -1) {
        perror("Failed to open source file");
        exit(EXIT_FAILURE);
    }

    int des_fd = open(des, O_CREAT | O_WRONLY, 0666);
    if (des_fd == -1) {
        perror("Failed to open destination file");
        close (src_fd);
        exit(EXIT_FAILURE);
    }

    char buffer[4096];
    ssize_t bytes_read;
    while ((bytes_read = read(src_fd, &buffer, sizeof(buffer))) != 0) {
        ssize_t bytes_written = write(des_fd, &buffer, bytes_read);
        if (bytes_written != bytes_read) {
            perror("Error writing to destination file");
            close(src_fd);
            close(des_fd);
            exit(EXIT_FAILURE);
        }
    }

    close(src_fd);
    close(des_fd);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "copy takes: " << duration << " (ms)" << std::endl;
}

int main() {
    copy("1GB_file_copy.bin", "des_1GB_file_copy.bin");
    zero_copy("1GB_file_zero_copy.bin", "des_1GB_file_zero_copy.bin");
    return 0;
}

