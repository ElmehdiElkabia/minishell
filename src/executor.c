#include "../include/minishell.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>




void	ft_error(char *str, int n)
{
	ft_putstr_fd(str, 2);
	exit(n);
}

void	ft_perror(char *str, int n)
{
	perror(str);
	exit(n);
}

void	ft_cmd(char *str, char **cmd, int n)
{
	if (!cmd || !*cmd)
	{
		ft_putendl_fd(str, 2);
		exit(n);
	}
	ft_putstr_fd(str, 2);
	ft_putendl_fd(cmd[0], 2);
	exit(n);
}

char	*get_full_path(t_envp *head)
{
	return find_value_env("PATH", &head);
}
char	*path_join(char *cmd, char *part_path)
{
	char	*path;
	char	*tmp;

	tmp = ft_strjoin(part_path, "/");
	if (!tmp)
		return (NULL);
	path = ft_strjoin(tmp, cmd);
	free(tmp);
	return (path);
}

char	*get_path(char *cmd, t_envp *envp)
{
	char	**path_argv;
	char	*path_dir;
	char	*path;
	int		i;

	path_dir = get_full_path(envp);
	if (!path_dir)
		return (NULL);
	path_argv = ft_split(path_dir, ':');
	if (!path_argv)
		return (NULL);
	i = 0;
	while (path_argv[i])
	{
		path = path_join(cmd, path_argv[i]);
		if (!path)
		{
			return (NULL);
		}
		if (access(path, F_OK) == 0)
		{
			return (path);
		}
		free(path);
		i++;
	}
	return (NULL);
}

char	**convert_envp(t_envp *envp)
{
	int		count = 0;
	t_envp	*tmp = envp;
	char	**envp_arr;

	while (tmp)
	{
		count++;
		tmp = tmp->next;
	}
	envp_arr = malloc((count + 1) * sizeof(char *));
	if (!envp_arr)
		return (NULL);
	tmp = envp;
	for (int i = 0; i < count; i++)
	{
		envp_arr[i] = malloc(strlen(tmp->key) + strlen(tmp->value) + 2);
		if (!envp_arr[i])
		{
			return (NULL);
		}
		sprintf(envp_arr[i], "%s=%s", tmp->key, tmp->value);
		tmp = tmp->next;
	}
	envp_arr[count] = NULL;
	return (envp_arr);
}

void	ft_hundel(char **cmd, t_envp *envp)
{
	char **envp_arr = convert_envp(envp);
	if (!envp_arr)
		ft_perror("Memory allocation failed", 1);

	if (ft_strchr(cmd[0], '/') != NULL)
	{
		if (execve(cmd[0], cmd, envp_arr) == -1)
		{
			ft_perror("Command execution failed", 126);
		}
	}
}


void execute_command(char *cmd, t_envp *head)
{
	char	**cmd_argv;
	char	*path;
	char	**envp_arr;

	cmd_argv = parse_input(cmd);
	if (!cmd_argv)
		ft_error("Memory allocation failed\n", 1);
	if (is_builtin(cmd_argv))
	{
		execute_builtin(cmd_argv, &head);
		return;
	}
	if (!cmd_argv[0])
	{
		ft_cmd("Command not found: ", cmd_argv, 127);
		return;
	}
	ft_hundel(cmd_argv, head);
	path = get_path(cmd_argv[0], head);
	if (!path)
	{
		ft_cmd("Command not found: ", cmd_argv, 127);
		return;
	}
	envp_arr = convert_envp(head);
	if (!envp_arr)
	{
		free(path);
		ft_error("Memory allocation failed\n", 1);
	}
	if (execve(path, cmd_argv, envp_arr) == -1)
	{
		free(path);
		ft_cmd("Execution failed: ", cmd_argv, 126);
	}
	free(path);
}
