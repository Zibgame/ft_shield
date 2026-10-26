/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   context.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zcadinot <zcadinot@student.42lehavre.      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/26 14:53:34 by zcadinot          #+#    #+#             */
/*   Updated: 2026/10/26 14:53:35 by zcadinot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_shield.h"

int check_context(void)
{
    unsigned char *peb;
    DWORD flag;
    BOOL remote;

    if (IsDebuggerPresent())
        return (1);
    peb = (unsigned char *)__readgsqword(0x60);
    if (peb[2])
        return (1);
    flag = *(DWORD *)(peb + 0x68);
    if (flag & 0x70)
        return (1);
    remote = 0;
    CheckRemoteDebuggerPresent(GetCurrentProcess(), &remote);
    if (remote)
        return (1);
    return (0);
}
