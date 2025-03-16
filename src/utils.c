/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eelkabia <eelkabia@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/15 14:15:31 by eelkabia          #+#    #+#             */
/*   Updated: 2025/03/16 16:15:40 by eelkabia         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"

char *ft_strdup_range(char *str, int start, int end)
{
	char *new_str;

	new_str = (char *)malloc(sizeof(char) * (end - start + 1));
	if (!new_str)
		return (NULL);
	memcpy(new_str, str + start, end - start);
	new_str[end - start] = '\0';
	return (new_str);
}

void free_tokens(char **tokens, int count)
{
    for (int i = 0; i < count; i++)
        free(tokens[i]);
    free(tokens);
}

char **parse_input(char *str)
{
	char **result;
	int i;
	int j;
	int start;
	char quote;

	i = 0;
	j = 0;
	result = (char **)malloc(100 * sizeof(char *));
	while (str[i])
	{
		while (isspace(str[i]))
			i++;
		if (!str[i])
			break;
		start = i;
		if (str[i] == '\'' || str[i] == '"')
		{
			quote = str[i++];
			start = i;
			while (str[i] && str[i] != quote)
				i++;
			if (str[i] == quote)
				result[j++] = ft_strdup_range(str, start, i++);
			else
			{
				fprintf(stderr, "Error: Unclosed quote\n");
				free_tokens(result, j);
				return (NULL);
			}
		}
		else
		{
			while (str[i] && !isspace(str[i]) && str[i] != '\'' && str[i] != '"')
				i++;
			result[j++] = ft_strdup_range(str, start, i);
		}
	}
	result[j] = NULL;
	return (result);
}
