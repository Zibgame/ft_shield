/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   kill.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zcadinot <zcadinot@student.42lehavre.      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/22 17:00:00 by zcadinot          #+#    #+#             */
/*   Updated: 2026/10/22 17:00:00 by zcadinot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_shield.h"
#include <stdio.h>
#include <string.h>
#include <tlhelp32.h>

static int kill_process(DWORD pid, char *self_name, DWORD self_pid)
{
    HANDLE process;

    (void)self_name;
    if (pid == self_pid)
        return (0);
    process = OpenProcess(PROCESS_TERMINATE | SYNCHRONIZE, FALSE, pid);
    if (process == NULL)
        return (0);
    if (!TerminateProcess(process, 0))
    {
        CloseHandle(process);
        return (0);
    }
    WaitForSingleObject(process, 3000);
    CloseHandle(process);
    return (1);
}

int mode_kill(void)
{
    HANDLE snapshot;
    PROCESSENTRY32 entry;
    char self[MAX_PATH];
    DWORD self_pid;
    int killed;

    killed = 0;
    self_pid = GetCurrentProcessId();
    if (GetModuleFileNameA(NULL, self, MAX_PATH) == 0)
        self[0] = '\0';
    task_uninstall();
    snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE)
        return (0);
    entry.dwSize = sizeof(entry);
    if (Process32First(snapshot, &entry))
    {
        do {
            if (strcmp(entry.szExeFile, "ft_shield.exe") == 0)
                killed += kill_process(entry.th32ProcessID, self, self_pid);
        } while (Process32Next(snapshot, &entry));
    }
    CloseHandle(snapshot);
    debug_printf("Killed %d instance(s)\n", killed);
    return (0);
}
