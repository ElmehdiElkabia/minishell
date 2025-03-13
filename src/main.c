/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eelkabia <eelkabia@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/08 01:34:39 by eelkabia          #+#    #+#             */
/*   Updated: 2025/03/13 16:24:19 by eelkabia         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"

char	*promot(void)
{
	char	*promot;
	char	*pwd;

	pwd = getcwd(NULL, 0);
	if (!pwd)
		return NULL;
	promot = ft_strjoin("$:", pwd);
	free(pwd);
	promot = ft_strjoin(promot , "> ");
	return (promot);
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
		data.str = readline(promot());
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