/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zcadinot <zcadinot@student.42lehavre.      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/19 18:02:57 by zcadinot          #+#    #+#             */
/*   Updated: 2026/10/19 18:17:02 by zcadinot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <windows.h>
#include <lmcons.h>
#include <stdio.h>

int main(int argc, char **argv)
{
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

