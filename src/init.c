/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eelkabia <eelkabia@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/08 01:55:25 by eelkabia          #+#    #+#             */
/*   Updated: 2025/03/13 16:16:09 by eelkabia         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"

void print_declare_x(t_envp **head)
{
	t_envp *temp;

	temp = (*head);
	while (temp)
	{
		printf("declare -x %s=\"%s\"\n", temp->key, temp->value);
		temp = temp->next;
	}
}

void export_env_variable(t_minishell *data, t_envp **head)
{
	char *equal_sign;
	char *key;
	int key_len;

	if (data->arg_cmd[1] == NULL)
		print_declare_x(head);
	equal_sign = ft_strchr(data->arg_cmd[1], '=');
	if (equal_sign)
	{
		key_len = equal_sign - data->arg_cmd[1];
		key = (char *)malloc(key_len + 1);
		if (!key)
			return;
		ft_memcpy(key, data->arg_cmd[1], key_len);
		if (find_value_env(key, head) != NULL)
			update_env_node(head, key, equal_sign + 1);
		else
			add_env_node(head, key, equal_sign + 1);
		free(key);
	}
}

void builtins_echo(t_minishell *data)
{
	int i;

	i = 1;
	while (data->arg_cmd[i])
	{
		printf("%s", data->arg_cmd[i]);
		if (data->arg_cmd[i + 1])
			printf(" ");
		i++;
	}
	printf("\n");
}

void builtins_echo_n(t_minishell *data)
{
	int i;

	i = 2;
	while (data->arg_cmd[i] && ft_strncmp(data->arg_cmd[i], "-n", 2) == 0)
		i++;
	while (data->arg_cmd[i])
	{
		printf("%s", data->arg_cmd[i]);
		if (data->arg_cmd[i + 1])
			printf(" ");
		i++;
	}
}

void exit_shell(t_minishell *data)
{
	int exit_code;
	int i;

	i = 0;
	exit_code = 0;
	if (data->arg_cmd[1])
	{
		while (data->arg_cmd[1][i])
		{
			if (!isdigit(data->arg_cmd[1][i]))
			{
				printf("minishell: exit: %s: numeric argument required\n", data->arg_cmd[1]);
				exit(255);
			}
			i++;
		}
		if (data->arg_cmd[2])
		{
			printf("minishell: exit: too many arguments\n");
			return;
		}
		exit_code = ft_atoi(data->arg_cmd[1]) % 256;
	}
	printf("exit\n");
	exit(exit_code);
}

void ft_getcwd(void)
{
	char *pwd;

	pwd = getcwd(NULL, 0);
	if (!pwd)
	{
		perror("pwd");
		return;
	}
	printf("%s\n", pwd);
	free(pwd);
}

void fr_cd(t_minishell *data, t_envp **head)
{
	char *path;
	char *new_pwd;
	char *old_pwd;

	old_pwd = getcwd(NULL, 0);
	if (!data->arg_cmd[1])
		path = getenv("HOME");
	else if (ft_strncmp(data->arg_cmd[1], "~", 1) == 0)
		path = getenv("OLDPWD");
	else
		path = data->arg_cmd[1];
	if (chdir(path) != 0)
	{
		perror("cd");
		free(old_pwd);
		return;
	}
	new_pwd = getcwd(NULL, 0);
	if (new_pwd)
	{
		update_env_node(head, "OLDPWD", old_pwd);
		update_env_node(head, "PWD", new_pwd);
		free(new_pwd);
	}
	free(old_pwd);
}

void execve_input(t_minishell *data, t_envp **head)
{
	data->arg_cmd = ft_split(data->str, ' ');
	if (!data->arg_cmd)
		return;
	else if (ft_strncmp(data->arg_cmd[0], "env", 4) == 0)
		print_env_list(*head);
	else if (ft_strncmp(data->arg_cmd[0], "unset", 6) == 0)
		remove_env_variable(data->arg_cmd[1], head);
	else if (ft_strncmp(data->arg_cmd[0], "export", 7) == 0)
		export_env_variable(data, head);
	else if (ft_strncmp(data->arg_cmd[0], "echo", 5) == 0)
	{
		if (data->arg_cmd[1] && ft_strncmp(data->arg_cmd[1], "-n", 2) == 0)
			builtins_echo_n(data);
		else
			builtins_echo(data);
	}
	else if (data->arg_cmd[0] && ft_strncmp(data->arg_cmd[0], "exit", 5) == 0)
		exit_shell(data);
	else if (ft_strncmp(data->arg_cmd[0], "pwd", 4) == 0)
		ft_getcwd();
	else if (ft_strncmp(data->arg_cmd[0], "cd", 3) == 0)
		fr_cd(data, head);
}
