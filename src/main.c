
#include "ft_shield.h"
#include <stdarg.h>
#include <stdio.h>

void debug_printf(const char *format, ...)
{
#ifdef DEBUG
    va_list args;

    va_start(args, format);
    vprintf(format, args);
    va_end(args);
#else
    (void)format;
#endif
}

int main(int argc, char **argv)
{
    int ret;

    if (AttachConsole(ATTACH_PARENT_PROCESS))
    {
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
        setvbuf(stdout, NULL, _IONBF, 0);
        setvbuf(stderr, NULL, _IONBF, 0);
    }
    if (argc == 2 && strcmp(argv[1], "--daemon") == 0)
        ret = mode_daemon();
    else if (argc == 2 && strcmp(argv[1], "--kill") == 0)
        ret = mode_kill();
    else
        ret = mode_normal();
    FreeConsole();
    return (ret);
}
