#include <fcntl.h>
#include <stdio.h>
#include <sys/select.h>
#include <unistd.h>

void print_buffer(char* buffer) {
    for (unsigned i = 0; i < 16; i++) {
        for (unsigned j = 0; j < 16; j++) {
            printf("%02x ", buffer[i * 16 + j]);
        }
        printf("\n");
    }
}

int main(int argc, char* argv[]) {
    char buffer[256];
    int fd = open("/dev/slave-eeprom", O_RDWR);
    if (fd < 0) {
        perror("open /dev/slave-eeprom");
        return -1;
    }
    read(fd, buffer, sizeof(buffer));
    lseek(fd, 0, SEEK_SET);
    print_buffer(buffer);
    while (1) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        int ret = select(fd + 1, &fds, NULL, NULL, NULL);
        if (ret) {
            // changed
            read(fd, buffer, sizeof(buffer));
            lseek(fd, 0, SEEK_SET);
            printf("changed\n");
            print_buffer(buffer);
        }
    }
    return 0;
}
