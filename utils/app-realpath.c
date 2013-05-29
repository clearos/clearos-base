#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

extern int errno;

int main(int argc, char *argv[])
{
    char *resolved_path = NULL;
    if(argc != 2) return EXIT_FAILURE;
    resolved_path = realpath(argv[1], NULL);
    if(!resolved_path)
    {
        fprintf(stderr, "%s: %s: %s\n",
            argv[0], argv[1], strerror(errno));
        return EXIT_FAILURE;
    }
    fprintf(stdout, "%s\n", resolved_path);
    free(resolved_path);
    return EXIT_SUCCESS;
}

// vi: expandtab shiftwidth=4 softtabstop=4 tabstop=4
