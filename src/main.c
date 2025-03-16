/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eelkabia <eelkabia@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/08 01:34:39 by eelkabia          #+#    #+#             */
/*   Updated: 2025/03/16 16:15:50 by eelkabia         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"

char	*prompt(void)
{
	char	*prompt;
	char	*pwd;

	pwd = getcwd(NULL, 0);
	if (!pwd)
		return NULL;
	prompt = ft_strjoin("$:", pwd);
	free(pwd);
	prompt = ft_strjoin(prompt , "> ");
	return (prompt);
}

int main(int argc, char **argv, char **envp)
{
	(void)argc;
	(void)argv;
	t_envp *env;
	t_minishell data;

	env = NULL;
	init_env(&env, envp);
	while (1)
	{
		data.str = readline(prompt());
		if (!data.str)
		{
			printf("exit\n");
			free_env_list(env);
			exit(0);
		}
		if (*data.str == '\0')
		{
			free(data.str);
			continue;
		}
		add_history(data.str);
		execve_input(&data, &env);
		free(data.str);
	}
	free_env_list(env);
	return (0);
}
