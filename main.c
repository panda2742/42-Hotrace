/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehosta <ehosta@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/28 15:37:28 by ehosta            #+#    #+#             */
/*   Updated: 2026/03/01 12:45:39 by ehosta           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "hotrace.h"

static void	dispatch0_(t_hm	*hm, char **tmp, char **line)
{
	if (!*tmp)
	{
		*tmp = *line;
		*line = get_next_line(0);
	}
	else
	{
		(*line)[ft_strlen(*line) - 1] = 0;
		(*tmp)[ft_strlen(*tmp) - 1] = 0;
		hm_insert(hm, *tmp, *line);
		*tmp = NULL;
		*line = get_next_line(0);
	}
}

static void	dispatch1_(t_hm	*hm, char **line)
{
	char	*res;

	if (ft_strlen(*line) == 1)
	{
		free(*line);
		*line = get_next_line(0);
		return ;
	}
	(*line)[ft_strlen(*line) - 1] = 0;
	res = hm_get(hm, *line);
	if (res)
	{
		write(1, res, ft_strlen(res));
		write(1, "\n", 1);
	}
	else
	{
		write(1, *line, ft_strlen(*line));
		write(1, NOT_FOUND_ERR, NOT_FOUND_ERR_LEN);
	}
	free(*line);
	*line = get_next_line(0);
}

static inline void	run_(t_hm *hm, char **tmp, char **line)
{
	int	mode;

	mode = 0;
	while (*line)
	{
		if (ft_strlen(*line) == 1)
		{
			if (mode == 0)
			{
				free(*line);
				mode = 1;
				*line = get_next_line(0);
				continue ;
			}
		}
		if (mode == 0)
			dispatch0_(hm, tmp, line);
		else if (mode == 1)
			dispatch1_(hm, line);
	}
}

int	main(void)
{
	t_hm	*hm;
	char	*tmp;
	char	*line;

	hm = hm_new(HM_INIT_CAP);
	if (!hm)
		return (1);
	tmp = NULL;
	line = get_next_line(0);
	run_(hm, &tmp, &line);
	if (tmp)
		free(tmp);
	if (line)
		free(line);
	hm_free(hm);
	return (0);
}
