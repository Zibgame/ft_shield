/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   directory.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zcadinot <zcadinot@student.42lehavre.      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/20 15:14:18 by zcadinot          #+#    #+#             */
/*   Updated: 2026/10/20 15:16:46 by zcadinot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_shield.h"

char *get_my_path(void)
{
    static char path[MAX_PATH];
    DWORD       length;

    length = GetModuleFileNameA(NULL, path, MAX_PATH);
    if (length > 0 && length < MAX_PATH)
        return (path);
    if (length >= MAX_PATH)
        SetLastError(ERROR_INSUFFICIENT_BUFFER);

    return (NULL);
}
