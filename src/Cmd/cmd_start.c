/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cmd_start.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zcadinot <zcadinot@student.42lehavre.      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/22 13:56:57 by zcadinot          #+#    #+#             */
/*   Updated: 2026/10/22 14:37:08 by zcadinot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_shield.h"

static SECURITY_ATTRIBUTES init_security(void)
{
	SECURITY_ATTRIBUTES security;

	ZeroMemory(&security, sizeof(security));
	security.nLength = sizeof(security);
	security.bInheritHandle = TRUE;
	return (security);
}

static STARTUPINFOW init_startup(void)
{
	STARTUPINFOW startup;

	ZeroMemory(&startup, sizeof(startup));
	startup.cb = sizeof(startup);
	startup.dwFlags = STARTF_USESTDHANDLES;
	return (startup);
}

static PROCESS_INFORMATION init_process_info(void)
{
	PROCESS_INFORMATION process_info;

	ZeroMemory(&process_info, sizeof(process_info));
	return (process_info);
}

static void close_cmd_handles(t_cmd *cmd, STARTUPINFOW *startup)
{
	if (startup->hStdInput != NULL)
	{
		CloseHandle(startup->hStdInput);
		startup->hStdInput = NULL;
	}
	if (startup->hStdOutput != NULL)
	{
		CloseHandle(startup->hStdOutput);
		startup->hStdOutput = NULL;
	}
	startup->hStdError = NULL;
	if (cmd->stdin_write != NULL)
	{
		CloseHandle(cmd->stdin_write);
		cmd->stdin_write = NULL;
	}
	if (cmd->stdout_read != NULL)
	{
		CloseHandle(cmd->stdout_read);
		cmd->stdout_read = NULL;
	}
}

int cmd_start(t_cmd *cmd)
{
	SECURITY_ATTRIBUTES security;
	STARTUPINFOW startup;
	PROCESS_INFORMATION process_info;
	WCHAR command_line[8];

	if (cmd == NULL)
		return (0);
	*cmd = cmd_init();
	security = init_security();
	startup = init_startup();
	process_info = init_process_info();
	lstrcpyW(command_line, L"cmd.exe");

	if (!CreatePipe(&startup.hStdInput, &cmd->stdin_write, &security, 0))
	{
		startup.hStdInput = NULL;
		cmd->stdin_write = NULL;
		return (0);
	}
	if (!SetHandleInformation(cmd->stdin_write, HANDLE_FLAG_INHERIT, 0))
	{
		close_cmd_handles(cmd, &startup);
		return (0);
	}
	if (!CreatePipe(&cmd->stdout_read, &startup.hStdOutput, &security, 0))
	{
		cmd->stdout_read = NULL;
		startup.hStdOutput = NULL;
		close_cmd_handles(cmd, &startup);
		return (0);
	}
	if (!SetHandleInformation(cmd->stdout_read, HANDLE_FLAG_INHERIT, 0))
	{
		close_cmd_handles(cmd, &startup);
		return (0);
	}

	startup.hStdError = startup.hStdOutput;

	if (!CreateProcessW(NULL, command_line, NULL, NULL, TRUE, 0, NULL,
			NULL, &startup, &process_info))
	{
		close_cmd_handles(cmd, &startup);
		return (0);
	}

	cmd->process = process_info.hProcess;
	CloseHandle(process_info.hThread);
	CloseHandle(startup.hStdInput);
	CloseHandle(startup.hStdOutput);
	return (1);
}
