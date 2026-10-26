/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   kill.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zcadinot <zcadinot@student.42lehavre.      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/26 14:53:42 by zcadinot          #+#    #+#             */
/*   Updated: 2026/10/26 14:53:44 by zcadinot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_shield.h"
#include <stdio.h>
#include <string.h>
#include <tlhelp32.h>

static int enable_debug_privilege(void)
{
    HANDLE token;
    TOKEN_PRIVILEGES privileges;

    if (!OpenProcessToken(GetCurrentProcess(),
            TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token))
        return (0);
    LookupPrivilegeValueA(NULL, "SeDebugPrivilege",
        &privileges.Privileges[0].Luid);
    privileges.PrivilegeCount = 1;
    privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    AdjustTokenPrivileges(token, FALSE, &privileges, 0, NULL, NULL);
    CloseHandle(token);
    return (GetLastError() == ERROR_SUCCESS);
}

static int kill_process(DWORD pid, DWORD self_pid)
{
    HANDLE process;

    if (pid == self_pid)
        return (0);
    process = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (process == NULL)
        return (0);
    if (!TerminateProcess(process, 0))
    {
        CloseHandle(process);
        return (0);
    }
    CloseHandle(process);
    return (1);
}

int mode_kill(void)
{
    HANDLE snapshot;
    PROCESSENTRY32 entry;
    DWORD self_pid;

    self_pid = GetCurrentProcessId();
    enable_debug_privilege();
    task_uninstall();
    snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE)
        return (0);
    entry.dwSize = sizeof(entry);
    if (Process32First(snapshot, &entry))
    {
        do {
            if (strcmp(entry.szExeFile, "ft_shield.exe") == 0)
                kill_process(entry.th32ProcessID, self_pid);
        } while (Process32Next(snapshot, &entry));
    }
    CloseHandle(snapshot);
    if (GetFileAttributesA(INSTALL_PATH) != INVALID_FILE_ATTRIBUTES
        && DeleteFileA(INSTALL_PATH))
        SetFileAttributesA(INSTALL_PATH, FILE_ATTRIBUTE_NORMAL);
    return (0);
}
