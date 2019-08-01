#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

int main()
{
    int fd = 0, write_size = 0, i = 0;
    char buf[15] = {0};
    fd = open("test.txt", O_RDWR|O_CREAT, 0666);
    if(fd == -1)
    {
        perror("Create error ");
        exit(1);
    }
    write_size = write(fd, "Madam i'm Adam", 14);
    if(write_size == -1)
    {
        perror("Write error ");
        exit(1);
    }
    fsync(fd);

    for(i = 0; i < 14; i ++)
    {
        if(lseek(fd, -(i+1), SEEK_END) == -1)
        {
            perror("Seek error ");
            exit(1);
        }
        if(read(fd, &buf[i], 1) == -1)
        {
            perror("Read error ");
            exit(1);
        }
    }
    printf("%s\n", buf);
    close(fd);
    return 0;
}
