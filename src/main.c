/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zcadinot <zcadinot@student.42lehavre.      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/19 18:02:57 by zcadinot          #+#    #+#             */
/*   Updated: 2026/10/20 15:08:59 by zcadinot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_sheild.h"
#include <lmcons.h>
#include <stdio.h>

int mode_daemon()
{
    printf("Mode_Daemon");
    while (1)
    {
        continue;
    }
    return (0);
}

int mode_normal()
{
    if (!is_elevated())
        return (0);

    char login[UNLEN + 1];
    DWORD size = sizeof(login);

    if (!GetUserNameA(login, &size))
        return (1);

    printf("%s\n", login);

    while (1)
    {
        continue;
    }
    return (0);
}

int main(int argc, char **argv)
{
    if (argc == 2 && strcmp(argv[1], "--daemon") == 0)
        return (mode_daemon());

    return (mode_normal());
}
