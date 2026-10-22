/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cmd.h                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zcadinot <zcadinot@student.42lehavre.      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/22 14:00:00 by zcadinot          #+#    #+#             */
/*   Updated: 2026/10/22 14:19:19 by zcadinot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <windows.h>

typedef struct s_cmd
{
    HANDLE process;
    HANDLE stdin_write;
    HANDLE stdout_read;
}   t_cmd;

t_cmd	cmd_init(void);
