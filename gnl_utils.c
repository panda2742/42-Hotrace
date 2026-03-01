/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   gnl_utils.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehosta <ehosta@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/13 15:17:50 by ehosta            #+#    #+#             */
/*   Updated: 2026/03/01 13:07:03 by ehosta           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "hotrace.h"

static inline void	ft_sstrlen_asm_(uint64_t *pos, uint64_t *mask)
{
	__asm__	volatile ("bsfq %1, %0"
			: "=r"(*pos)
			: "r"(*mask)
			: "cc");
}

t_size	ft_sstrlen(const char *str)
{
	const char		*s = str;
	const uint64_t	*ptr;
	uint64_t		word;
	uint64_t		mask;
	uint64_t		pos;

	while (((uintptr_t)s & 7) && *s)
		s++;
	if (!*s)
		return (s - str);
	ptr = (const uint64_t *)s;
	while (1)
	{
		word = *ptr++;
		mask = (word - 0x0101010101010101UL) & ~word & 0x8080808080808080UL;
		if (mask)
		{
			ft_sstrlen_asm_(&pos, &mask);
			pos >>= 3;
			return ((const char *)ptr - str - 8 + pos);
		}
	}
}

static inline void	ft_memmove_asm1_(char *dst, const char *src, t_size len)
{
	__asm__ volatile (
		"xorq   %%rcx, %%rcx            \n\t"
		"1:                             \n\t"
		"cmpq   $8, %2                  \n\t"
		"jl     2f                      \n\t"
		"movq   (%1, %%rcx), %%rax      \n\t"
		"movq   %%rax, (%0, %%rcx)      \n\t"
		"addq   $8, %%rcx               \n\t"
		"subq   $8, %2                  \n\t"
		"jmp    1b                      \n\t"
		"2:                             \n\t"
		"testq  %2, %2                  \n\t"
		"je     3f                      \n\t"
		"movb   (%1, %%rcx), %%al       \n\t"
		"movb   %%al, (%0, %%rcx)       \n\t"
		"incq   %%rcx                   \n\t"
		"decq   %2                      \n\t"
		"jmp    2b                      \n\t"
		"3:                             \n\t"
		: "+r"(dst), "+r"(src), "+r"(len)
		:
		: "rcx", "rax", "memory"
	);
}

static inline void	ft_memmove_asm2_(char *dst, const char *src, t_size len)
{
	__asm__ volatile (
		"movq   %2, %%rcx               \n\t"
		"1:                             \n\t"
		"cmpq   $8, %%rcx               \n\t"
		"jl     2f                      \n\t"
		"subq   $8, %%rcx               \n\t"
		"movq   (%1, %%rcx), %%rax      \n\t"
		"movq   %%rax, (%0, %%rcx)      \n\t"
		"jmp    1b                      \n\t"
		"2:                             \n\t"
		"testq  %%rcx, %%rcx            \n\t"
		"je     3f                      \n\t"
		"decq   %%rcx                   \n\t"
		"movb   (%1, %%rcx), %%al       \n\t"
		"movb   %%al, (%0, %%rcx)       \n\t"
		"jmp    2b                      \n\t"
		"3:                             \n\t"
		: "+r"(dst), "+r"(src), "+r"(len)
		:
		: "rcx", "rax", "memory"
	);
}

void	*ft_memmove(char *dst, const char *src, t_size len)
{
	if (dst == src || len == 0)
		return (dst);
	if (dst < src)
		ft_memmove_asm1_(dst, src, len);
	else
		ft_memmove_asm2_(dst, src, len);
	return (dst);
}
