/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   redirections.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eelkabia <eelkabia@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/07 08:32:18 by eelkabia          #+#    #+#             */
/*   Updated: 2025/04/07 09:11:01 by eelkabia         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"

void check_redirections(char *str)
{
	int i;
	int fd;
	char **token;

	token = parse_input(str);
	i = 0;
	while (token[i])
	{
		if (strncmp(token[i], ">", 1) == 0 && token[i][1] == '\0' && token[i + 1])
		{
			fd = open(token[i + 1], O_CREAT | O_WRONLY | O_TRUNC, 0644);
			if (fd < 0)
				perror("open");
			else
				dup2(fd, STDOUT_FILENO);
		}
		else if (strncmp(token[i], ">>", 2) == 0 && token[i][2] == '\0' && token[i + 1])
		{
			fd = open(token[i + 1], O_CREAT | O_WRONLY | O_APPEND, 0644);
			if (fd < 0)
				perror("open");
			else
				dup2(fd, STDOUT_FILENO);
		}
		else if (strncmp(token[i], "<", 1) == 0 && token[i][1] == '\0' && token[i + 1])
		{
			fd = open(token[i + 1], O_RDONLY);
			if (fd < 0)
				perror("open");
			else
				dup2(fd, STDIN_FILENO);
		}
		i++;
	}
}