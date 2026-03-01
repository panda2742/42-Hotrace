/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   memutils.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehosta <ehosta@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/28 14:40:22 by ehosta            #+#    #+#             */
/*   Updated: 2026/03/01 13:03:19 by ehosta           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "hotrace.h"

static void	ft_bzero_(void *s, t_size n)
{
	const t_size	n_divided = n / 16;
	const t_size	n_moduloed = n % 16;
	unsigned char	*p;
	t_size			i;

	i = 0;
	while (i < n_divided)
		*((unsigned __int128 *)s + i++) = (unsigned __int128)0;
	i ^= i;
	p = (unsigned char *)s + n_divided * 16;
	while (i < n_moduloed)
		p[i++] = 0;
}

void	*ft_calloc(t_size count, t_size size)
{
	t_size	bytes;
	void	*ptr;

	bytes = count * size;
	if (count != 0 && (count * size) / count != size)
		return (NULL);
	ptr = malloc(bytes);
	if (!ptr)
		return (NULL);
	ft_bzero_(ptr, bytes);
	return (ptr);
}

t_size	ft_strlen(const char *s)
{
	t_size	i;

	i = 0;
	while (s[i])
		i++;
	return (i);
}
