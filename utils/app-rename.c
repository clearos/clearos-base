///////////////////////////////////////////////////////////////////////////////
//
// Copyright 2000 Point Clark Networks.
//
// This software may be freely redistributed under the terms of the GNU
// public license.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
///////////////////////////////////////////////////////////////////////////////
//
// Wrapper to do privileged stuff from the web-based admininistration tool
//
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <crypt.h>
#include <shadow.h>

int main(int argc, char *argv[]) {
    size_t bytes;
    char *buffer;
    int fd_s, fd_d;
    struct stat buf;

    if (argc <= 2) {
        return 1;
    }

    setgroups(0, NULL);
    setgid(0);
    setuid(0);

    if (stat(argv[2], &buf) != 0) {
        perror("stat");
        return 1;
    }

    if((fd_s = open(argv[1], O_RDONLY)) < 0) {
        perror("open");
        return 1;
    }

    if((fd_d = open(argv[2], O_WRONLY | O_TRUNC, buf.st_mode)) < 0) {
        perror("open");
        return 1;
    }

    // Copy file argv[1] -> argv[2]...
    if(!(buffer = malloc(getpagesize()))) {
        perror("malloc");
        return 1;
    }

    while((bytes = read(fd_s, buffer, getpagesize())) > 0)
        write(fd_d, buffer, bytes);

    close(fd_s);
    close(fd_d);

    free(buffer);

    // Keep the permissions of the target file
    if (chmod(argv[2], buf.st_mode) != 0) {
        perror("chmod");
        return 1;
    }

    if (chown(argv[2], buf.st_uid, buf.st_gid) != 0) {
        perror("chown");
        return 1;
    }

    unlink(argv[1]);

    return 0;
}

// vi: expandtab shiftwidth=4 softtabstop=4 tabstop=4
