/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   encryption.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zcadinot <zcadinot@student.42lehavre.      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/26 14:53:51 by zcadinot          #+#    #+#             */
/*   Updated: 2026/10/26 14:53:53 by zcadinot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_shield.h"

static int sha256_string(const char *input, char output[65])
{
	BCRYPT_ALG_HANDLE algorithm;
	unsigned char result[32];
	const char *hex;
	unsigned long i;
	NTSTATUS status;

	if (input == NULL || output == NULL)
		return (0);
	algorithm = NULL;
	hex = "0123456789abcdef";
	status = BCryptOpenAlgorithmProvider(
		&algorithm,
		BCRYPT_SHA256_ALGORITHM,
		NULL,
		0);
	if (status != 0)
		return (0);
	status = BCryptHash(
		algorithm,
		NULL,
		0,
		(PUCHAR)input,
		(ULONG)strlen(input),
		result,
		sizeof(result));
	BCryptCloseAlgorithmProvider(algorithm, 0);
	if (status != 0)
		return (0);
	i = 0;
	while (i < sizeof(result))
	{
		output[i * 2] = hex[result[i] >> 4];
		output[i * 2 + 1] = hex[result[i] & 0x0f];
		i++;
	}
	output[64] = '\0';
	return (1);
}

int verify_access(const char *password)
{
	char hash[65];

	return (sha256_string(password, hash)
		&& strcmp(hash, PASSWORD_SHA256) == 0);
}
