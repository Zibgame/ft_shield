#ifndef FT_SHIELD_H
#define FT_SHIELD_H

#include <sys/types.h>

#define PORT 4242
#define MAX_CLIENTS 3
#define PASSWORD_HASH "5988450b67c8cbb3a0d444273a95d376"

int run_daemon(void);
int setup_persistence(void);

#endif
