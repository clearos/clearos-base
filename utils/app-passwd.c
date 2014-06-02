// app-passwd: PAM authentication application for Webconfig
// Copyright (C) 2014 ClearFoundation <http://www.clearfoundation.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <security/pam_appl.h>

// Max user and password, choosing arbitrary value
#define MAX_USERPASS    128

// PAM application name
#define PAM_APP_NAME    "system-auth-ac"

// Input buffer
static char buffer[MAX_USERPASS * 2];

// User, and password buffers
static char user[MAX_USERPASS], pass[MAX_USERPASS];

// PAM private application data structure
struct app_data
{
    char *user;
    char *pass;
};

// Global application data
static struct app_data ad = {
    .user = NULL,
    .pass = NULL,
};

// Zero memory buffers
void reset(void)
{
    memset(user, 0, MAX_USERPASS);
    memset(pass, 0, MAX_USERPASS);
    memset(buffer, 0, MAX_USERPASS * 2);

    if (ad.user != NULL) free(ad.user);
    if (ad.pass != NULL) free(ad.pass);
}

// PAM conversation call-back
int app_conv(int num_msg, const struct pam_message **msgm,
    struct pam_response **response, void *app_data_ptr)
{
    int i;
    struct app_data *ad = (struct app_data *)app_data_ptr;
    struct pam_response *resp = calloc(num_msg, sizeof(struct pam_response));

    for (i = 0; i < num_msg; i++) {

        resp[i].resp_retcode = 0;

        switch (msgm[i]->msg_style) {

        // Want user name
        case PAM_PROMPT_ECHO_ON:
            resp[0].resp = ad->user;
            break;

        // Want password
        case PAM_PROMPT_ECHO_OFF:
            resp[0].resp = ad->pass;
            break;

        // Un-handled request...
        default:
            free(resp);
            return PAM_CONV_ERR;
        }
    }

    *response = resp;
    ad->user = NULL;
    ad->pass = NULL;

    return PAM_SUCCESS;
}

// Global PAM conversation
static struct pam_conv conv = {
    app_conv,
    (void *)&ad
};

int main(int argc, char *argv[])
{
    int i, j, rc;
    pam_handle_t *pamh = NULL;

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

    ad.user = strdup(user);
    ad.pass = strdup(pass);

    rc = pam_start(PAM_APP_NAME, ad.user, &conv, &pamh);

    if (rc == PAM_SUCCESS)
        rc = pam_authenticate(pamh, 0);

    if (rc == PAM_SUCCESS)
        rc = pam_acct_mgmt(pamh, 0);

    if (pam_end(pamh,rc) != PAM_SUCCESS) {
        pamh = NULL;
        return 1;
    }

    return (rc == PAM_SUCCESS ? 0 : 1);
}

// vi: expandtab shiftwidth=4 softtabstop=4 tabstop=4
