
#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/types.h>

#include <unistd.h>
#include <sched.h>

void usage()
{
    printf("setns\nusage: setns pid program [OPTIONS]\n");
}

int main(int argc, char* argv[])
{
    int i;
    int fd;
    int rc;
    char* path;
    char** newargv;
    int s;
	int newargc = argc - 1;

    if (argc < 3)
    {
        usage();

        return -1;
    }

    s = snprintf(path, 0, "/proc/%s/ns/net", argv[1]);

    if (s < 0)
    {
        printf("%s", strerror(errno));

        return -1;
    }

    ++s;
    path = malloc(s);

    if (path == NULL)
    {
        printf("Error allocating memory\n");

        return -1;
    }

    snprintf(path, s, "/proc/%s/ns/net", argv[1]);

    fd = open(path, O_RDONLY);
           
    if (fd == -1)
    {
        printf("open error: %s\n", strerror(errno));

        return -1;
    }

    rc = setns(fd, CLONE_NEWNET);
    close(fd);
    free(path);

    if (rc == -1)
    {
        printf("setns error: %s\n", strerror(errno));

        return -1;
    }

    newargv = malloc(newargc * sizeof(char*));

    for (i=0; i<newargc; ++i)
    {
        newargv[i] = argv[i + 2];
    }

    newargv[newargc - 1] = NULL;

    execvp(argv[2], newargv);

    printf("execve error: %s\n", strerror(errno));

    return -1;
}
