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
// General synopsis:
// -Accepts username password pair (seperated by white-space) from standard in.
// -Looks up username in shadow password file.
// -Extracts salt from password entry.
// -Crypts supplied password with extracted salt.
// -Returns 0 if crypted passwords are identical.
// -Returns 1 if anything goes wrong along the way.
//
// Compilation:
// # gcc -O2 -s -o cc-passwd -lcrypt cc-passwd.c
//
///////////////////////////////////////////////////////////////////////////////
// $Id: cc-passwd.c,v 1.1.1.1 2004/01/16 03:41:51 devel Exp $

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <crypt.h>
#include <shadow.h>

// Max salt size is 16 chars + $x[x]$ + NULL
#define MAX_SALT    (16 + 3 + 1)

// Max user and password, choosing arbitrary value
#define MAX_USERPASS 128

// Input buffer
static char buffer[MAX_USERPASS * 2];

// Salt, user, and password buffers
static char salt[MAX_SALT], user[MAX_USERPASS], pass[MAX_USERPASS];

// Zero memory buffers
void reset(void)
{
    memset(salt, '\0', MAX_SALT);
    memset(user, '\0', MAX_USERPASS);
    memset(pass, '\0', MAX_USERPASS);
    memset(buffer, '\0', MAX_USERPASS * 2);
}

int main(int argc, char *argv[])
{
    int i, j;
    struct spwd *passwd;
    int salt_pos = -1, salt_len = 0;

    atexit(reset); reset();

    if (fread(buffer, 1, MAX_USERPASS * 2, stdin) < 3)
        return 1;

    for (i = 0, j = 0; i < MAX_USERPASS - 1; i++) {
        if (isspace(buffer[i])) { i++; break; }
        if (!isalpha(buffer[i]) &&
            !isdigit(buffer[i]) &&
            !ispunct(buffer[i])) continue;
        user[j++] = buffer[i];
    }

    for (j = 0; i < (MAX_USERPASS - 1) * 2 &&
        j < MAX_USERPASS - 1; i++) {
        if (buffer[i] == '\n' || buffer[i] == '\r') break;
        if (!isalpha(buffer[i]) &&
            !isdigit(buffer[i]) &&
            !ispunct(buffer[i]) &&
            !isspace(buffer[i])) continue;
        pass[j++] = buffer[i];
    }

    if (!strnlen(user, MAX_USERPASS - 1) || !strnlen(pass, MAX_USERPASS - 1))
        return 1;

    if (!(passwd = getspnam(user)))
        return 1;

    if (passwd->sp_pwdp == NULL)
        return 1;

    if (passwd->sp_pwdp[0] != '$')
        return 1;

    if (passwd->sp_pwdp[2] == '$')
        salt_pos = 2;
    else if (passwd->sp_pwdp[3] == '$')
        salt_pos = 3;
    else
        return 1;

    for (salt_len = salt_pos + 1;
        salt_len < salt_pos + MAX_SALT - 1; salt_len++) {
        if (passwd->sp_pwdp[salt_len] == '\0' ||
            passwd->sp_pwdp[salt_len] == '$') break;
    }

    strncpy(salt, passwd->sp_pwdp, salt_len);
    strncpy(pass, crypt(pass, salt), MAX_USERPASS - 1);

    if (!strncmp(pass, passwd->sp_pwdp, MAX_USERPASS - 1))
        return 0;

    return 1;
}

// vi: expandtab shiftwidth=4 softtabstop=4 tabstop=4
