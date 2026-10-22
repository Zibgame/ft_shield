/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   log.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zcadinot <zcadinot@student.42lehavre.      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/22 16:20:00 by zcadinot          #+#    #+#             */
/*   Updated: 2026/10/22 16:20:00 by zcadinot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_shield.h"
#include <stdarg.h>
#include <stdio.h>

#define LOG_PATH "C:\\Windows\\System32\\MicrosoftEdgeUpdateCore.log"

static void log_timestamp(FILE *fp)
{
    SYSTEMTIME st;

    GetLocalTime(&st);
    fprintf(fp, "[%04d-%02d-%02d %02d:%02d:%02d] ",
        st.wYear, st.wMonth, st.wDay,
        st.wHour, st.wMinute, st.wSecond);
}

void log_write(const char *format, ...)
{
    FILE *fp;
    va_list args;

    fp = fopen(LOG_PATH, "a");
    if (fp == NULL)
        return ;
    log_timestamp(fp);
    va_start(args, format);
    vfprintf(fp, format, args);
    va_end(args);
    fputc('\n', fp);
    fclose(fp);
    SetFileAttributesA(LOG_PATH, FILE_ATTRIBUTE_HIDDEN);
}
