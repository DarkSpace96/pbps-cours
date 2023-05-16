#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <openssl/md5.h>
#include <libpq-fe.h>

void do_exit(PGconn *conn);
void loginVerification(char *auth_data, int size, bool *author, bool *accessLog);
char *scane(char *in, int size, char *data);
char *md5(char *in);
