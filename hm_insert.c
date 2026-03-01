/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   hm_insert.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehosta <ehosta@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/28 12:34:57 by ehosta            #+#    #+#             */
/*   Updated: 2026/03/01 14:44:00 by ehosta           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "hotrace.h"

static int	hm_reinsert_(t_slot *old, t_size old_cap, t_slot *new, t_size mask)
{
	t_size	i;
	t_size	idx;

	i = 0;
	while (i < old_cap)
	{
		if (old[i].key && old[i].key != (char *)1)
		{
			idx = old[i].hash & mask;
			while (new[idx].key)
				idx = (idx + 1) & mask;
			new[idx] = old[i];
		}
		i++;
	}
	return (0);
}

static int	hm_resize_(t_hm *hm)
{
	t_size	new_cap;
	t_slot	*new_slots;
	t_slot	*old_slots;

	new_cap = hm->capacity * 2;
	new_slots = ft_calloc(new_cap, sizeof(t_slot));
	if (!new_slots)
		return (-1);
	old_slots = hm->slots;
	hm_reinsert_(old_slots, hm->capacity, new_slots, new_cap - 1);
	free(old_slots);
	hm->slots = new_slots;
	hm->capacity = new_cap;
	hm->mask = new_cap - 1;
	hm->dead = 0;
	return (0);
}

static void	hm_probe_(t_hm *hm, t_size *idx, t_size *tomb, int *has_tomb)
{
	if (hm->slots[*idx].key == (char *)1 && !*has_tomb)
	{
		*has_tomb = 1;
		*tomb = *idx;
	}
	*idx = (*idx + 1) & hm->mask;
}

static inline t_size	calc_idx_(t_size idx, t_size has_tomb, t_size tomb_idx)
{
	if (has_tomb)
		return (tomb_idx);
	return (idx);
}

int	hm_insert(t_hm *hm, char *key, char *value)
{
	uint32_t	h;
	t_size		idx;
	t_size		tomb_idx;
	int			has_tomb;

	if ((hm->count + hm->dead + 1) * HM_LOAD_DEN > hm->capacity * HM_LOAD_NUM)
		if (hm_resize_(hm) < 0)
			return (-1);
	h = hash_fnv1a(key);
	idx = h & hm->mask;
	has_tomb = 0;
	tomb_idx = 0;
	while (1)
	{
		if (!hm->slots[idx].key)
		{
			idx = calc_idx_(idx, has_tomb, tomb_idx);
			hm->slots[idx] = (t_slot){key, value, h};
			return (++hm->count, 0);
		}
		if (hm->slots[idx].hash == h && hm->slots[idx].key != (char *)1
			&& simd_strcmp(hm->slots[idx].key, key))
			return (erase_value(hm, idx, key, value));
		hm_probe_(hm, &idx, &tomb_idx, &has_tomb);
	}
}
