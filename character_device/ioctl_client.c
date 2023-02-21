#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define IOCTL_LOGIN  _IO('m', 1)

struct msg_struct {
    char username[256];
    char message[256];
};

struct login_struct {
    char username[256];
    char tty_name[256];
};

int main()
{
    char username[256];
    printf("Enter username: ");
    fscanf(stdin, "%s", username);

    char *tty_name;
    tty_name = ttyname(STDIN_FILENO);
    printf("Current tty: %s\n", tty_name);

    int fd = open("/dev/my_chrdev", O_RDWR);
    if (fd < 0) {
        perror("Failed to open device file");
        return -1;
    }

    if (ioctl(fd, IOCTL_LOGIN) < 0) {
        printf("ioctl failed");
        return -1;
    } else {
        printf("Logged in!");
    }

    close(fd);

    return 0;
}
