/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cmd_io.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zcadinot <zcadinot@student.42lehavre.      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/22 14:32:34 by zcadinot          #+#    #+#             */
/*   Updated: 2026/10/22 14:47:28 by zcadinot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_shield.h"

#define CMD_END_MARKER "__CMD_OUTPUT_END__"

static int send_all(SOCKET slot, const char *buffer, int length)
{
	int total;
	int sent;

	total = 0;
	while (total < length)
	{
		sent = send(slot, buffer + total, length - total, 0);
		if (sent == SOCKET_ERROR || sent == 0)
			return (0);
		total += sent;
	}
	return (1);
}

static int cmd_write(t_cmd *cmd, const char *buffer, DWORD length)
{
	DWORD bytes_written;

	if (!WriteFile(cmd->stdin_write, buffer, length,
			&bytes_written, NULL))
		return (0);
	if (bytes_written != length)
		return (0);
	return (1);
}

int cmd_exec(t_cmd *cmd, const char *command)
{
	DWORD command_length;
	const char marker_command[] =
		"\r\necho " CMD_END_MARKER "\r\n";

	if (cmd == NULL || cmd->stdin_write == NULL || command == NULL)
		return (0);
	command_length = lstrlenA(command);
	if (!cmd_write(cmd, command, command_length))
		return (0);
	if (!cmd_write(cmd, marker_command, sizeof(marker_command) - 1))
		return (0);
	return (1);
}

int cmd_send_output(t_cmd *cmd, SOCKET slot)
{
	char line[4096];
	char character;
	char *marker;
	DWORD bytes_read;
	int length;

	if (cmd == NULL || cmd->stdout_read == NULL)
		return (0);
	length = 0;
	while (1)
	{
		if (!ReadFile(cmd->stdout_read, &character, 1,
				&bytes_read, NULL))
		{
			if (GetLastError() == ERROR_BROKEN_PIPE)
				return (1);
			return (0);
		}
		if (bytes_read == 0)
			return (1);
		line[length++] = character;
		if (character == '\n' || length == sizeof(line) - 1)
		{
			line[length] = '\0';
			marker = strstr(line, CMD_END_MARKER);
			if (marker != NULL)
			{
				if (marker > line
					&& !send_all(slot, line, (int)(marker - line)))
					return (0);
				return (1);
			}
			if (!send_all(slot, line, length))
				return (0);
			length = 0;
		}
	}
}
