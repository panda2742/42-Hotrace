/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   gnl.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehosta <ehosta@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/13 15:14:50 by ehosta            #+#    #+#             */
/*   Updated: 2026/03/01 13:11:34 by ehosta           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "hotrace.h"

ssize_t	get_nl_pos(const char *s, int len)
{
	ssize_t	i;

	i = 0;
	while (s[i] && s[i] != '\n')
		i++;
	if (!s[i])
		return ((-1 * (len == 0)) + (i * len));
	return (i);
}

char	*copy_and_move(char *res, char *buffer, ssize_t eof_buffer, char *out)
{
	t_size	i;
	ssize_t	j;

	i = 0;
	j = 0;
	while (res && res[j])
		out[i++] = res[j++];
	j = 0;
	while (j < eof_buffer)
		out[i++] = buffer[j++];
	out[i] = 0;
	ft_memmove(
		buffer, buffer + eof_buffer, ft_sstrlen(buffer + eof_buffer) + 1);
	if (res)
		free(res);
	return (out);
}

char	*join_res_buffer(char *res, char *buffer, int *end, int is_nl)
{
	ssize_t	eof_buffer;
	t_size	res_len;
	t_size	out_len;
	char	*out;

	res_len = 0;
	eof_buffer = get_nl_pos(buffer, 1);
	if (!is_nl && !res && !eof_buffer)
		return (NULL);
	if (is_nl)
	{
		*end = 1;
		eof_buffer++;
	}
	if (res)
		res_len = ft_sstrlen(res);
	out_len = res_len + eof_buffer;
	out = malloc(out_len + 1);
	if (!out && res)
		free(res);
	if (!out)
		return (NULL);
	res = copy_and_move(res, buffer, eof_buffer, out);
	return (out);
}

char	*get_next_line(int fd)
{
	static char	buffer[FOPEN_MAX][BUFFER_SIZE + 1];
	char		*res;
	ssize_t		read_bytes;
	int			end;

	if (fd < 0 || BUFFER_SIZE <= 0 || fd >= FOPEN_MAX)
		return (NULL);
	end = 0;
	res = join_res_buffer(
			NULL, buffer[fd], &end, get_nl_pos(buffer[fd], 0) > -1);
	read_bytes = 1;
	while (read_bytes > 0 && !end)
	{
		read_bytes = read(fd, buffer[fd], BUFFER_SIZE);
		if (read_bytes < 0)
			buffer[fd][0] = 0;
		if (read_bytes < 0)
			return (free(res), NULL);
		if (read_bytes == 0)
			return (res);
		buffer[fd][read_bytes] = 0;
		res = join_res_buffer(
				res, buffer[fd], &end, get_nl_pos(buffer[fd], 0) > -1);
	}
	return (res);
}
