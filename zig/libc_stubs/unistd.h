// Stub unistd.h for freestanding pico-sdk builds
#ifndef _UNISTD_H
#define _UNISTD_H

#include <stddef.h>

// Standard file descriptors
#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

// Seek constants
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

// Type definitions
typedef int ssize_t;
typedef int off_t;
typedef int pid_t;
typedef unsigned int uid_t;
typedef unsigned int gid_t;

// Function stubs (these would need to be implemented for actual I/O)
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);
int close(int fd);
off_t lseek(int fd, off_t offset, int whence);
int isatty(int fd);

// Sleep functions
unsigned int sleep(unsigned int seconds);
int usleep(unsigned int usec);

// Exit functions
void _exit(int status);

#endif
