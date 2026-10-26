/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   is_elevated.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zcadinot <zcadinot@student.42lehavre.      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/26 14:53:27 by zcadinot          #+#    #+#             */
/*   Updated: 2026/10/26 14:53:29 by zcadinot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_shield.h"

int is_elevated(void)
{
    HANDLE token = NULL;
    TOKEN_ELEVATION elevation = {0};
    DWORD size = 0;
    BOOL success;

    if (!OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY, &token))
        return (-1);

    success = GetTokenInformation(
        token,
        TokenElevation,
        &elevation,
        sizeof(elevation),
        &size
    );

    CloseHandle(token);

    if (!success)
        return (-1);

    return (elevation.TokenIsElevated != 0);
}
