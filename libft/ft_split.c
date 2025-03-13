/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_split.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eelkabia <eelkabia@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/26 15:50:00 by eelkabia          #+#    #+#             */
/*   Updated: 2024/11/05 13:25:12 by eelkabia         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static	size_t	count_word(char	*str, char c)
{
	size_t	i;
	size_t	count;

	i = 0;
	count = 0;
	while (str[i] != '\0')
	{
		if ((i == 0 || str[i - 1] == c) && str[i] != c)
			count++;
		i++;
	}
	return (count);
}

static	char	**ft_free(char **result, size_t word_index)
{
	while (word_index > 0)
	{
		free(result[word_index - 1]);
		word_index--;
	}
	free(result);
	return (NULL);
}

static	char	*word(const char *s, size_t n)
{
	size_t	i;
	char	*dup ;

	i = 0;
	dup = (char *)malloc((n + 1) * sizeof(char));
	if (!dup)
		return (NULL);
	while (i < n)
	{
		dup[i] = s[i];
		i++;
	}
	dup[n] = '\0';
	return (dup);
}

static	char	**split_dup(char **result, const char *s, char c)
{
	size_t	i;
	size_t	start;
	size_t	word_index;

	i = 0;
	word_index = 0;
	start = 0;
	while (s[i])
	{
		if (s[i] != c)
		{
			start = i;
			while (s[i] && s[i] != c)
				i++;
			result[word_index] = word(s + start, i - start);
			if (!result[word_index])
				return (ft_free(result, word_index));
			word_index++;
		}
		else
			i++;
	}
	result[word_index] = NULL;
	return (result);
}

char	**ft_split(char const *s, char c)
{
	char	**result;
	size_t	count;

	if (!s)
		return (0);
	count = count_word((char *)s, c);
	result = (char **)malloc(sizeof(char *) * (count + 1));
	if (!result)
		return (NULL);
	result = split_dup(result, s, c);
	return (result);
}
