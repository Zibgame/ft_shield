/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zcadinot <zcadinot@student.42lehavre.      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/19 18:02:57 by zcadinot          #+#    #+#             */
/*   Updated: 2026/10/22 17:00:00 by zcadinot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
    /* GUI app (-mwindows) has no console. Attach back to the parent's one
       so the login is still visible when launched from a terminal,
       while a double-click opens nothing. */
    if (AttachConsole(ATTACH_PARENT_PROCESS))
    {
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
    }
    if (argc == 2 && strcmp(argv[1], "--daemon") == 0)
        return (mode_daemon());
    if (argc == 2 && strcmp(argv[1], "--kill") == 0)
        return (mode_kill());

    return (mode_normal());
}
