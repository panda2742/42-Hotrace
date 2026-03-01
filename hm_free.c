/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   hm_free.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehosta <ehosta@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/28 14:50:57 by ehosta            #+#    #+#             */
/*   Updated: 2026/03/01 11:37:15 by ehosta           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "hotrace.h"

void	hm_free(t_hm *hm)
{
	t_size	i;

	if (!hm)
		return ;
	if (hm->slots)
	{
		i = 0;
		while (i < hm->capacity)
		{
			free(hm->slots[i].key);
			free(hm->slots[i].value);
			++i;
		}
		free(hm->slots);
	}
	free(hm);
}
