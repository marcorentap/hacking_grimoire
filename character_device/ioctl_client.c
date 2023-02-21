#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>

#define IOCTL_SET_USER  _IO('m', 1)
char username[256];

void print_menu() {
    printf(
        "Select an option: \n"
        "1. Retrieve messages\n"
        "2. Send a message\n"
        "3. Exit\n"
    );
}

void retrieve_messages() {
    int fd = open("/dev/my_chrdev", O_RDWR);

    if (fd < 0) {
        perror("Failed to open device file");
        return  1;
    }
    
    // Receive 10 messages
    char buffer[10 * 1024];
    read(fd, buffer, 10 * 1024);
    printf("Received messages: \n%s\n", buffer);

    close(fd);
}

void send_message() {
    char msg_buffer[1024];
    printf("Enter your message: \n");
    fgets(msg_buffer, 1024, stdin);

    int fd = open("/dev/my_chrdev", O_RDWR);
    if (fd < 0) {
        perror("Failed to open device file");
        return  1;
    }

    if (ioctl(fd, IOCTL_SET_USER, username) < 0) {
        printf("ioctl failed\n");
        return -1;
    }

    write(fd, msg_buffer, 1024);

    close(fd);
}

int main()
{
    system("clear");
    printf("Enter username: ");
    fgets(username, 256, stdin);
    username[strcspn(username, "\n")] = 0;

    while(1) {
        print_menu();
        int opt;
        char buffer[8];
        fgets(buffer, 8, stdin);
        opt = atoi(buffer);

        switch (opt) {
            case 1:
                retrieve_messages();
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

        printf("\nEnter to continue...");
        fgets(buffer, 8, stdin);
        system("clear");
    }

    return 0;
}
