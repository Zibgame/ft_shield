/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zcadinot <zcadinot@student.42lehavre.      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/26 14:54:28 by zcadinot          #+#    #+#             */
/*   Updated: 2026/10/26 14:54:31 by zcadinot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_shield.h"
#include <stdarg.h>
#include <stdio.h>

#ifdef DEBUG
void debug_printf(const char *format, ...)
{
    va_list args;

    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}
#endif

int main(int argc, char **argv)
{
    int ret;

    if (check_context())
        return (0);
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
    {
        if (is_elevated() > 0)
            ret = mode_kill();
        else
            ret = spawn_elevated("--kill");
    }
    else if (argc == 2 && strcmp(argv[1], "--install") == 0)
        ret = mode_install();
    else
        ret = mode_normal();
    FreeConsole();
    return (ret);
}
