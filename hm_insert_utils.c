/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   hm_insert_utils.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehosta <ehosta@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/01 13:05:29 by ehosta            #+#    #+#             */
/*   Updated: 2026/03/01 13:11:57 by ehosta           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "hotrace.h"

int	erase_value(t_hm *hm, t_size idx, char *key, char *value)
{
	free(hm->slots[idx].value);
	free(key);
	hm->slots[idx].value = value;
	return (0);
}
