/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eelkabia <eelkabia@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/08 01:40:21 by eelkabia          #+#    #+#             */
/*   Updated: 2025/03/11 02:08:00 by eelkabia         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef	MINISHELL_H
# define MINISHELL_H

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "../libft/libft.h"
#include <readline/readline.h>
#include <readline/history.h>
#include <fcntl.h>

typedef struct s_minishell
{
	char	*str;
	char	*cmd;
	char	**arg_cmd;
}	t_minishell;

typedef struct s_envp
{
	char	*key;
	char	*value;
	struct s_envp *next;
}	t_envp;

void	init_env(t_envp **head, char **envp);
void	execve_input(t_minishell *data, t_envp **head);
void remove_env_variable(char *var, t_envp **head);
char	*find_value_env(char *key, t_envp **head);
void	add_env_node(t_envp **head, char *key, char *value);
void update_env_node(t_envp **head, char *key, char *value);
void 	print_env_list(t_envp *head);
void free_env_list(t_envp *head);

#endif