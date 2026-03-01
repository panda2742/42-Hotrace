/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   hm_utils.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehosta <ehosta@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/28 10:09:11 by ehosta            #+#    #+#             */
/*   Updated: 2026/02/28 16:11:11 by ehosta           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "hotrace.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <emmintrin.h>

static inline int	simd_strcmp_asm_(const char *a, const char *b, int *end,
	int *eq)
{
	__asm__	volatile (
			"movdqu     (%2), %%xmm0        \n\t"
			"movdqu     (%3), %%xmm1        \n\t"
			"pxor       %%xmm2, %%xmm2      \n\t"
			"pcmpeqb    %%xmm2, %%xmm0      \n\t"
			"pmovmskb   %%xmm0, %0          \n\t"
			"movdqu     (%2), %%xmm0        \n\t"
			"pcmpeqb    %%xmm1, %%xmm0      \n\t"
			"pmovmskb   %%xmm0, %1          \n\t"
			: "=r"(*end), "=r"(*eq)
			: "r"(a), "r"(b)
			: "xmm0", "xmm1", "xmm2"
			);

	return (*end);
}

int	simd_strcmp(const char *a, const char *b)
{
	int	eq;
	int	end;
	int	null_pos;
	int	relevant;

	while (1)
	{
		if (((uintptr_t)a & 0xFFF) > 4080 || ((uintptr_t)b & 0xFFF) > 4080)
		{
			while (*a && *(a++) == *(b++))
				;
			return (*a == *b);
		}
		if (simd_strcmp_asm_(a, b, &end, &eq))
		{
			__asm__ ("bsfl %1, %0" : "=r"(null_pos) : "r"(end));
			relevant = (1 << (null_pos + 1)) - 1;
			return ((eq & relevant) == relevant);
		}
		if (eq != 0xFFFF)
			return (0);
		a += 16;
		b += 16;
	}
}

t_size	next_pow2(t_size n)
{
	unsigned long long	highest_bit;

	if (n == 0)
		return (1);
	if ((n & (n - 1)) == 0)
		return (n);
	n--;
	__asm__ ("bsrq %1, %0" : "=r"(highest_bit) : "r"((unsigned long long)n));
	return (1UL << (highest_bit + 1));
}

char	*hm_get(t_hm *hm, const char *key)
{
	uint32_t	h;
	t_size		idx;

	h = hash_fnv1a(key);
	idx = h & hm->mask;
	while (1)
	{
		if (!hm->slots[idx].key)
			return (NULL);
		if (hm->slots[idx].key != (char *)1
			&& hm->slots[idx].hash == h
			&& simd_strcmp(hm->slots[idx].key, key))
			return (hm->slots[idx].value);
		idx = (idx + 1) & hm->mask;
	}
}

t_hm	*hm_new(t_size initial_capacity)
{
	t_hm	*hm;

	hm = malloc(sizeof(t_hm));
	if (!hm)
		return (NULL);
	if (initial_capacity)
		hm->capacity = next_pow2(initial_capacity);
	else
		hm->capacity = next_pow2(HM_INIT_CAP);
	hm->mask = hm->capacity - 1;
	hm->count = 0;
	hm->dead = 0;
	hm->slots = ft_calloc(hm->capacity, sizeof(t_slot));
	if (!hm->slots)
	{
		free(hm);
		return (NULL);
	}
	return (hm);
}
