#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define MY_IOCTL_CMD  _IO('m', 1)

int main()
{
    int fd = open("/dev/my_chrdev", O_RDWR);
    if (fd < 0) {
        perror("Failed to open device file");
        return -1;
    }

    if (ioctl(fd, MY_IOCTL_CMD) < 0) {
        perror("ioctl failed");
        return -1;
    }

    close(fd);
    return 0;
}

