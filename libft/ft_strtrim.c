/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strtrim.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eelkabia <eelkabia@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/24 22:39:03 by eelkabia          #+#    #+#             */
/*   Updated: 2024/11/11 15:50:22 by eelkabia         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	*ft_strtrim(char const *s1, char const *set)
{
	size_t	i;
	size_t	j;
	char	*str;

	i = 0;
	if (!s1)
		return (NULL);
	j = ft_strlen(s1);
	if (!set)
		return (ft_strdup(s1));
	while (s1[i] && ft_strchr(set, s1[i]))
		i++;
	while (i < j && ft_strchr(set, s1[j - 1]))
		j--;
	str = (char *)malloc((j - i + 1) * sizeof(char));
	if (!str)
		return (NULL);
	ft_strlcpy(str, s1 + i, j - i + 1);
	return (str);
}
