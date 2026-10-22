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

int     is_elevated(void);
char    *get_my_path(void);
int     task_install(const char *path, const char *arg);
int     persistence_task_detect(void);
