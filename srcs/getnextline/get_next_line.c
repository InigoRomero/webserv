/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iromero- <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/11/16 11:52:16 by iromero-          #+#    #+#             */
/*   Updated: 2019/11/18 15:06:08 by iromero-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "get_next_line.h"
#include <unistd.h>
#include <stdlib.h>

static	int	getcharpls(char **line, char **aux, int n)
{
	char *temp;
	char *temp2;

	if (n < 0)
		return (-1);
	if (!n && !*aux)
	{
		*line = ft_strdup("");
		return (0);
	}
	if ((temp = ft_strchr(*aux, '\n')))
	{
		*temp = 0;
		*line = ft_strdup(*aux);
		temp2 = ft_strdup(++temp);
		free(*aux);
		*aux = temp2;
	}
	else
	{
		*line = *aux;
		*aux = NULL;
		return (0);
	}
	return (1);
}

int			get_next_line(int fd, char **line)
{
	char			buffer[BUFFER_SIZE + 1];
	static char		*aux[4096];
	int				n;
	char			*temp;

	if (fd < 0 || BUFFER_SIZE < 1 || !line)
		return (-1);
	while ((n = read(fd, buffer, BUFFER_SIZE)) > 0)
	{
		buffer[n] = 0;
		if (!aux[fd])
			aux[fd] = ft_strdup(buffer);
		else
		{
			temp = ft_strjoin(aux[fd], buffer);
			free(aux[fd]);
			aux[fd] = temp;
		}
		if (ft_strchr(buffer, '\n'))
			break ;
	}
	return (getcharpls(line, &aux[fd], n));
}
