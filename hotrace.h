/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   hotrace.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehosta <ehosta@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/28 10:11:24 by ehosta            #+#    #+#             */
/*   Updated: 2026/03/01 14:45:25 by ehosta           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HOTRACE_H
# define HOTRACE_H

# include <stddef.h>
# include <inttypes.h>
# include <stdlib.h>
# include <stdlib.h>
# include <string.h>
# include <stdint.h>
# include <emmintrin.h>
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>

# define NOT_FOUND_ERR ": Not found.\n"
# define NOT_FOUND_ERR_LEN 14

/**
 * Load factor: the hashmap is resized when the amount of entries exceeds 70%
 * of the capacity (table size).
 */
# define HM_LOAD_NUM 7
# define HM_LOAD_DEN 10
# define HM_INIT_CAP 1048576 // 2^17

typedef unsigned long	t_size;

typedef struct s_slot
{
	char		*key;
	char		*value;
	uint32_t	hash;
}				t_slot;

typedef struct s_hm
{
	t_slot	*slots;
	/**
	 * Always a pow of 2.
	 */
	t_size	capacity;
	/**
	 * Capacity - 1.
	 */
	t_size	mask;
	/**
	 * Alive entries.
	 */
	t_size	count;
	/**
	 * Dead slots, tombs. Counted separately.
	 */
	t_size	dead;
}			t_hm;

# define STR_FRAG_BUFF_SIZE 256

typedef struct s_strfrag
{
	char				buffer[STR_FRAG_BUFF_SIZE];
	struct s_strfrag	*next;
}						t_strfrag;

static inline uint32_t	hash_fnv1a(const char *key)
{
	uint32_t	h;

	h = 2166136261u;
	__asm__ volatile (
		"xorl   %%eax, %%eax        \n\t"
		"1:                         \n\t"
		"movb   (%1), %%al          \n\t"
		"testb  %%al, %%al          \n\t"
		"jz     2f                  \n\t"
		"xorl   %%eax, %0           \n\t"
		"imull  $16777619, %0, %0   \n\t"
		"leaq   1(%1), %1           \n\t"
		"jmp    1b                  \n\t"
		"2:                         \n\t"
		: "+r"(h), "+r"(key)
		:
		: "eax"
	);
	return (h);
}

t_hm	*hm_new(t_size initial_capacity);
int		hm_insert(t_hm *hm, char *key, char *value);
char	*hm_get(t_hm *hm, const char *key);
void	hm_free(t_hm *hm);
int		simd_strcmp(const char *a, const char *b);
t_size	next_pow2(t_size n);
void	*ft_calloc(t_size count, t_size size);
t_size	ft_strlen(const char *s);
int		erase_value(t_hm *hm, t_size idx, char *key, char *value);

# ifndef BUFFER_SIZE
#  define BUFFER_SIZE 4096
# endif

char	*get_next_line(int fd);

t_size	ft_sstrlen(const char *s);
void	*ft_memmove(char *dst, const char *src, t_size len);

#endif
