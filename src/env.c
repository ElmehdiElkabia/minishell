/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eelkabia <eelkabia@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/10 21:13:26 by eelkabia          #+#    #+#             */
/*   Updated: 2025/03/11 02:20:15 by eelkabia         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"

t_envp *create_env_node(char *key, char *value)
{
	t_envp *node;

	node = (t_envp *)malloc(sizeof(t_envp));
	if (!node)
		return (NULL);
	node->key = ft_strdup(key);
	node->value = ft_strdup(value);
	node->next = NULL;
	return (node);
}

void add_env_node(t_envp **head, char *key, char *value)
{
	t_envp *node;

	node = create_env_node(key, value);
	if (!node)
		return;
	node->next = *head;
	*head = node;
}

void update_env_node(t_envp **head, char *key, char *value)
{
	t_envp *temp;

	temp = *head;
	while (temp)
	{
		if (ft_strncmp(temp->key, key, ft_strlen(key)) == 0 && ft_strlen(temp->key) == ft_strlen(key))
		{
			free(temp->value);
			temp->value = ft_strdup(value);
			return;
		}
		temp = temp->next;
	}
}

void print_env_list(t_envp *head)
{
	while (head)
	{
		printf("%s=%s\n", head->key, head->value);
		head = head->next;
	}
}

void free_env_list(t_envp *head)
{
	t_envp *tmp;
	while (head)
	{
		tmp = head;
		head = head->next;
		free(tmp->key);
		free(tmp->value);
		free(tmp);
	}
}

void init_env(t_envp **head, char **envp)
{
	int i;
	char *equal_sign;
	char *key;
	int key_len;

	i = 0;
	while (envp[i])
	{
		equal_sign = ft_strchr(envp[i], '=');
		if (equal_sign)
		{
			key_len = equal_sign - envp[i];
			key = (char *)malloc(key_len + 1);
			if (!key)
				return;
			ft_memcpy(key, envp[i], key_len);
			add_env_node(head, key, equal_sign + 1);
			free(key);
		}
		i++;
	}
}

char *find_value_env(char *key, t_envp **head)
{
	t_envp *temp;

	temp = *head;
	while (temp != NULL)
	{
		if (ft_strlen(key) == ft_strlen(temp->key) \
		&& ft_strncmp(key, temp->key, ft_strlen(key)) == 0)
			return temp->value;
		temp = temp->next;
	}
	return (NULL);
}

void remove_env_variable(char *var, t_envp **head)
{
	t_envp *temp = *head;
	t_envp *prev = NULL;

	while (temp != NULL && ft_strncmp(temp->key, var, ft_strlen(var)) != 0)
	{
		prev = temp;
		temp = temp->next;
	}

	if (temp != NULL)
	{
		if (prev == NULL)
			*head = temp->next;
		else
			prev->next = temp->next;

		free(temp->key);
		free(temp->value);
		free(temp);
	}
}