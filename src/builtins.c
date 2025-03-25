#include "../include/minishell.h"

int is_builtin(char **args)
{
    if (!args[0])
        return 0;
    if (!ft_strncmp(args[0], "env", 4)) return 1;
    if (!ft_strncmp(args[0], "pwd", 4)) return 1;
    if (!ft_strncmp(args[0], "cd", 3)) return 1;
    if (!ft_strncmp(args[0], "export", 7)) return 1;
    if (!ft_strncmp(args[0], "unset", 6)) return 1;
    if (!ft_strncmp(args[0], "echo", 5)) return 1;
    if (!ft_strncmp(args[0], "exit", 5)) return 1;
    return 0;
}

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

void export_env_variable(char **args, t_envp **head)
{
	char *equal_sign;
	char *key;
	int key_len;

	if (args[1] == NULL)
		print_declare_x(head);
	equal_sign = ft_strchr(args[1], '=');
	if (equal_sign)
	{
		key_len = equal_sign - args[1];
		key = (char *)malloc(key_len + 1);
		if (!key)
			return;
		ft_memcpy(key, args[1], key_len);
		if (find_value_env(key, head) != NULL)
			update_env_node(head, key, equal_sign + 1);
		else
			add_env_node(head, key, equal_sign + 1);
		free(key);
	}
}

void builtins_echo(char **args)
{
	int i;

	i = 1;
	while (args[i])
	{
		printf("%s", args[i]);
		if (args[i + 1])
			printf(" ");
		i++;
	}
	printf("\n");
}

void builtins_echo_n(char **args)
{
	int i;

	i = 2;
	while (args[i] && ft_strncmp(args[i], "-n", 2) == 0)
		i++;
	while (args[i])
	{
		printf("%s", args[i]);
		if (args[i + 1])
			printf(" ");
		i++;
	}
}

void exit_shell(char **args)
{
	int exit_code;
	int i;

	i = 0;
	exit_code = 0;
	if (args[1])
	{
		while (args[1][i])
		{
			if (!isdigit(args[1][i]))
			{
				printf("minishell: exit: %s: numeric argument required\n", args[1]);
				exit(255);
			}
			i++;
		}
		if (args[2])
		{
			printf("minishell: exit: too many arguments\n");
			return;
		}
		exit_code = ft_atoi(args[1]) % 256;
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

void fr_cd(char **args, t_envp **head)
{
	char *path;
	char *new_pwd;
	char *old_pwd;

	old_pwd = getcwd(NULL, 0);
	if (!args[1])
		path = getenv("HOME");
	else if (ft_strncmp(args[1], "~", 1) == 0)
		path = getenv("OLDPWD");
	else
		path = args[1];
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

void execute_builtin(char **args, t_envp **head)
{
    if (!ft_strncmp(args[0], "env", 4))
        print_env_list(*head);
    else if (!ft_strncmp(args[0], "pwd", 4))
        ft_getcwd();
    else if (!ft_strncmp(args[0], "cd", 3))
        fr_cd(args, head);
    else if (!ft_strncmp(args[0], "export", 7))
        export_env_variable(args, head);
    else if (!ft_strncmp(args[0], "unset", 6))
        remove_env_variable(args[1], head);
    else if (!ft_strncmp(args[0], "echo", 5))
        builtins_echo(args);
    else if (!ft_strncmp(args[0], "exit", 5))
        exit_shell(args);
}