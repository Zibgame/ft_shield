/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cmd_init.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zcadinot <zcadinot@student.42lehavre.      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/22 13:56:36 by zcadinot          #+#    #+#             */
/*   Updated: 2026/10/22 14:19:27 by zcadinot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_shield.h"

t_cmd cmd_init(void)
{
    t_cmd cmd;

    cmd.process = NULL;
    cmd.stdin_write = NULL;
    cmd.stdout_read = NULL;
    return (cmd);
}
