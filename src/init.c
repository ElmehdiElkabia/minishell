/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eelkabia <eelkabia@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/08 01:55:25 by eelkabia          #+#    #+#             */
/*   Updated: 2025/04/07 13:46:00 by eelkabia         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"



void ft_free_array(char **str)
{
	int i;

	i = 0;
	while (str[i])
	{
		free(str[i]);
		i++;
	}
	free(str);
}

// void execve_input(t_minishell *data, t_envp **head)
// {
// 	int	i;

// 	i = 0;
// 	data->arg_cmd = ft_split(data->str, '|');
// 	if (!data->arg_cmd)
// 		return;
// 	while (data->arg_cmd[i])
// 	{
// 		if (ft_strncmp(data->arg_cmd[i], "env", 4) == 0)
// 			print_env_list(*head);
// 		else if (ft_strncmp(data->arg_cmd[i], "unset", 6) == 0)
// 			remove_env_variable(data->arg_cmd[1], head);
// 		else if (ft_strncmp(data->arg_cmd[i], "export", 7) == 0)
// 			export_env_variable(data, head);
// 		else if (ft_strncmp(data->arg_cmd[i], "echo", 5) == 0)
// 		{
// 			if (data->arg_cmd[1] && ft_strncmp(data->arg_cmd[1], "-n", 2) == 0)
// 				builtins_echo_n(data);
// 			else
// 				builtins_echo(data);
// 		}
// 		else if (data->arg_cmd[i] && ft_strncmp(data->arg_cmd[i], "exit", 5) == 0)
// 			exit_shell(data);
// 		else if (ft_strncmp(data->arg_cmd[i], "pwd", 4) == 0)
// 			ft_getcwd();
// 		else if (ft_strncmp(data->arg_cmd[i], "cd", 3) == 0)
// 			fr_cd(data, head);
// 		else if (ft_strncmp(data->arg_cmd[i], "clear", 6) == 0)
// 			printf("\033[H\033[J");
// 		i++;
// 	}
// 	if (data->arg_cmd)
// 		ft_free_array(data->arg_cmd);
// }

char	*strip_redirections(char *cmd)
{
	// printf("cmd ==> %s\n", cmd);
	char	**tokens = ft_split(cmd, ' ');
	char	*new_cmd = NULL;
	char	*tmp;
	int		i = 0;

	while (tokens[i])
	{
		// printf("tokens[%d] ==> %s\n", i, tokens[i]);
		if (strcmp(tokens[i], ">") == 0 || strcmp(tokens[i], "<") == 0 || strcmp(tokens[i], ">>") == 0)
		{
			i += 2;
			continue;
		}
		tmp = new_cmd;
		if (!new_cmd)
			new_cmd = ft_strdup(tokens[i]);
		else
		{
			new_cmd = ft_strjoin(new_cmd, " ");
			free(tmp);
			tmp = new_cmd;
			new_cmd = ft_strjoin(new_cmd, tokens[i]);
			free(tmp);
		}
		i++;
	}
	return (new_cmd);
}



void	ft_process(char *str, t_command *cmd, int input_fd, t_envp **head)
{
	pid_t	id;
	char	*str_cmd = strip_redirections(str);
	id = fork();
	if (id == 0)
	{
		if (input_fd != 0)
		{
			dup2(input_fd, 0);
			close(input_fd);
		}
		if (cmd->args[1])
		{
			dup2(cmd->pipe_fd[1], 1);
			close(cmd->pipe_fd[1]);
		}
		// close(cmd->pipe_fd[0]);
		check_redirections(str);
		execute_command(str_cmd, (*head));
	}
}

void wait_for_children(int num_children)
{
	int	status;
	int	i;

	i = 0;
	while (i < num_children)
	{
		wait(&status);
		i++;
	}
}

void execve_input(t_minishell *data, t_envp **head)
{
	t_command	cmd;
	int			i;
	int			input_fd;

	data->arg_cmd = ft_split(data->str, '|');
	if (!data->arg_cmd)
		return ;
	i = 0;
	input_fd = 0;
	while (data->arg_cmd[i])
	{
		if (data->arg_cmd[i + 1])
			pipe(cmd.pipe_fd);
		ft_process(data->arg_cmd[i], &cmd, input_fd, head);
		if (data->arg_cmd[i + 1])
			close(cmd.pipe_fd[1]);
		input_fd = cmd.pipe_fd[0];
		i++;
	}
	wait_for_children(i);
}
