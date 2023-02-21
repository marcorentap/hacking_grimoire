#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>

#define IOCTL_LOGIN  _IO('m', 1)
char username[256];

void print_menu() {
    printf(
        "Select an option: \n"
        "1. Retrieve messages\n"
        "2. Send a message\n"
        "3. Exit"
    );
}

void retrieve_messages() {
    printf("received something woohoo");
}

void send_message(int fd) {
    char msg_buffer[1024];
    print("Enter your message: ");
    fgets(msg_buffer, 1024, stdin);

    int fd = open("/dev/my_chrdev", O_RDWR);
    if (fd < 0) {
        perror("Failed to open device file");
        return -1;
    }

    if (ioctl(fd, IOCTL_SET_USER, username) < 0) {
        printf("ioctl failed");
        return -1;
    } else {
        printf("Set user to %s\n", username);
    }
    fprintf(fd, msg_buffer);

    close(fd);
}

int main()
{
    printf("Enter username: ");
    fscanf(stdin, "%s", username);

    
    int opt;
    fscanf(stdin, "%d", opt);

    switch (opt) {
        case 1:
            retrieve_message();
            break;
        case 2:
            send_message();
            break;
        case 3:
            exit(1);
            break;
        default:
            printf("Invalid option\n");
            continue;
    }
    return 0;
}
