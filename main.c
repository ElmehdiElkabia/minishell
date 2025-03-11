#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <dirent.h>
#include <linux/limits.h>
#include <readline/readline.h>
#include <readline/history.h>


//step 1, create a program that enters, keep the history no matter what and then exit
/*step 2, Determine the token type based on the current character.
    If it's a quote (single or double), read until the closing quote.
    If it's a redirection operator (like >, >>, <, <<), determine the type.
    If it's a pipe (|), add as a pipe token.
Otherwise, it's part of a word; collect until a whitespace or special character is encountered.
*/

// echo (-n), cd, pwd, export, unset, env, exit, clear

// printf must be remplaced by ft_printf

typedef enum e_role
{
    CMD,      // Regular command
    PIPE,     //  |
    REDIR_IN, //  <
    HDOC,     //  <<
    TRUNC,    //  >
    APPEND    //  >>
}   t_role;

typedef struct s_env
{
    char			*var;
    char			*value;
    struct s_env	*next;
}                   t_env;


void	lst_clean(t_env *head)
{
    t_env *tmp;
    while (head)
    {
        tmp = head;
        head = head->next;
        free(tmp->var);
        free(tmp->value);
        free(tmp);
    }
}

void	free_array(char **arr)
{
	int	i;

	if (!arr)
		return;
	i = 0;
	while (arr[i])
	{
		free(arr[i]);
		i++;
	}
	free(arr);
}

size_t	ft_strlcpy(char *dst, const char *src, size_t dstsize)
{
	size_t	i;
	size_t	j;

	i = 0;
	j = 0;
	if (!src)
		src = "";
	if (!dst)
		return (0);
	while (src[i])
		i++;
	if (dstsize == 0)
		return (i);
	while (src[j] && j < dstsize - 1)
	{
		dst[j] = src[j];
		j++;
	}
	dst[j] = '\0';
	return (i);
}

int	ft_strlen(const char *s)
{
	int	i;

	if (!s)
		return (0);
	i = 0;
	while (s[i])
		i++;
	return (i);
}

char	*ft_strdup(const char *s1)
{
	char	*t1;
	int		i;

	i = 0;
	if (!s1)
		return (NULL);
	t1 = malloc((ft_strlen(s1) + 1) * sizeof(char));
	if (!t1)
		return (NULL);
	while (s1[i])
	{
		t1[i] = s1[i];
		i++;
	}
	t1[i] = '\0';
	return (t1);
}

static void free_argv(char **argv, int count)
{
    for (int i = 0; i < count; i++)
        free(argv[i]);
    free(argv);
}

static int count_args(const char *input)
{
    int count = 0;
    int in_quotes = 0;
    char quote_char = 0;

    while (*input)
    {
        while (*input == ' ')
            input++;
        if (!*input)
            break;
        count++;
        while (*input && (in_quotes || *input != ' '))
        {
            if ((*input == '\'' || *input == '"') && !in_quotes)
            {
                in_quotes = 1;
                quote_char = *input;
            }
            else if (*input == quote_char && in_quotes)
            {
                in_quotes = 0;
                quote_char = 0;
            }
            input++;
        }
    }
    return (count);
}

int	ft_strcmp(char *str1, const char *str2)
{
    while (*str1 && (*str1 == *str2))
	{
        str1++;
        str2++;
    }
    return (*(unsigned char *)str1 - *(unsigned char *)str2);
}

static char *get_next_arg(const char **input)
{
    const char *start = *input;
    int in_quotes = 0;
    char quote_char = 0;

    while (**input == ' ')
        (*input)++;
    start = *input;
    while (**input && (in_quotes || **input != ' '))
    {
        if ((**input == '\'' || **input == '"') && !in_quotes)
        {
            in_quotes = 1;
            quote_char = **input;
        }
        else if (**input == quote_char && in_quotes)
        {
            in_quotes = 0;
            quote_char = 0;
        }
        (*input)++;
    }
    int len = *input - start;
    char *arg = malloc(len + 1);
    if (!arg)
        return (NULL);
    strncpy(arg, start, len);
    arg[len] = '\0';
    return (arg);
}

char **parse_prompt_to_argv(const char *input)
{
    if (!input)
        return (NULL);
    int argc = count_args(input);
    if (argc == 0)
        return (NULL);
    char **argv = malloc((argc + 1) * sizeof(char *));
    if (!argv)
        return (NULL);
    for (int i = 0; i < argc; i++)
    {
        argv[i] = get_next_arg(&input);
        if (!argv[i])
        {
            free_argv(argv, i);
            return (NULL);
        }
    }
    argv[argc] = NULL;
    return (argv);
}

void	error_par(char **data, const char *msg)
{
	write(2, msg, ft_strlen(msg));
	if (data)
		free_array(data);
	exit(1);
}

void	ft_lstadd_back(t_env **lst, t_env *new)
{
	t_env	*temp;

	if (!lst)
		return ;
	if (!(*lst))
		*lst = new;
	else
	{
		temp = *lst;
		while (temp->next)
			temp = temp->next;
		temp->next = new;
	}
}

char	*ft_substr(char const *s, unsigned int start, size_t len)
{
	size_t		i;
	char		*t;

	if (!s)
		return (NULL);
	if (start >= ft_strlen(s))
		return (ft_strdup(""));
	i = 0;
	if (len > ft_strlen(s) - start)
		len = ft_strlen(s) - start;
	t = malloc((len + 1) * sizeof(char));
	if (!t)
		return (NULL);
	while (s[start + i] && i < len)
	{
		t[i] = s[start + i];
		i++;
	}
	t[i] = '\0';
	return (t);
}

t_env *create_node(char *line)
{
    t_env   *new_node;

	new_node = malloc(sizeof(t_env));
    if (!new_node)
        return NULL;
    if (!line)
	{
        free(new_node);
        return NULL;
    }
    if (strchr(line, '='))
    {
        new_node->var = ft_substr(line, 0, strchr(line, '=') - line);
        new_node->value = ft_strdup(strchr(line, '=') + 1);
    }
    if (!new_node->var || !new_node->value)
    {
        free(new_node->var);
        free(new_node->value);
        free(new_node);
        return NULL;
    }
    new_node->next = NULL;
    return (new_node);
}

t_env *create_env(char **envp)
{
    int     i;
    int     flag;
    t_env   *lst;
    t_env   *current;
	t_env   *new_node;

	i = 0;
    flag = 0;
	lst = NULL;
    if (!envp)
        return (NULL);
    while (envp[i])
    {
        if (strstr(envp[i], "HOME="))
            flag = 1;
        new_node = create_node(envp[i]);
        if (!new_node)
        {
            printf("Warning: Failed to create environment node for: %s\n", envp[i]);
            i++;
            continue;
        }
        if (!lst)
        {
            lst = new_node;
            current = lst;
        }
        else
        {
            current->next = new_node;
            current = current->next;
        }
        i++;
    }
    if (flag == 0)
    {
        new_node = create_node("HOME=/home/izahr");
        if (!new_node)
        {
            printf("Warning: Failed to create environment node for: %s\n", envp[i]);
            i++;
        }
        if (!lst)
        {
            lst = new_node;
            current = lst;
        }
        else
        {
            current->next = new_node;
            current = current->next;
        }
    }
    return (lst);
}

void	parse_env(t_env *lst, int k)
{
	if (!lst)
	{
		write(1, "There is No Environement Variables. . .\n", 41);
		exit(1);
	}
	while (lst)
	{
		if (k == 0)
			printf("%s=%s\n", lst->var, lst->value);
		else
			printf("declare -x %s=\"%s\"\n", lst->var, lst->value);
		lst = lst->next;
	}
}

t_env *clear_node(t_env *env, char *str)
{
    t_env *current;
    t_env *tmp;

	tmp = NULL;
	current = env;
    if (current && !ft_strcmp(current->var, str))
    {
        env = current->next;
        free(current->var);
        free(current->value);
        free(current);
        return env;
    }
    while (current)
    {
        if (!ft_strcmp(current->var, str))
        {
            if (tmp)
                tmp->next = current->next;
            free(current->var);
            free(current->value);
            free(current);
            break;
        }
        tmp = current;
        current = current->next;
    }

    return (env);
}

char *check_env(t_env *env, char *str)
{
    t_env *tmp = env;

    if (*str == '$')
        str++;
    if (*str == '\'' || *str == '"')
        str++;
    char *var_name_end = str;
    while (*var_name_end && *var_name_end != '\'' && *var_name_end != '"')
        var_name_end++;
    char saved_char = *var_name_end;
    *var_name_end = '\0';
    while (tmp)
    {
        if (!ft_strcmp(tmp->var, str))
        {
            *var_name_end = saved_char;
            return tmp->value;
        }
        tmp = tmp->next;
    }
    *var_name_end = saved_char;
    return NULL;
}

char *special_check(const char *token, t_env *env)
{
    size_t len = strlen(token);
    char *result = malloc(len + 1);
    if (!result)
        return NULL;
    size_t i = 0, j = 0;
    int in_double_quotes = 0;
    int in_single_quotes = 0;

    while (token[i]) {
        if (token[i] == '"' && !in_single_quotes)
        {
            in_double_quotes = !in_double_quotes;
            i++;
        }
        else if (token[i] == '\'' && !in_double_quotes)
        {
            in_single_quotes = !in_single_quotes;
            i++;
        }
        else if (token[i] == '$' && !in_single_quotes)
        {
            // expand variable: simple implementation scanning until non-alphanumeric/underscore.
            i++;
            size_t var_start = i;
            while (token[i] && ((token[i] >= 'A' && token[i] <= 'Z') ||
                                (token[i] >= 'a' && token[i] <= 'z') ||
                                (token[i] >= '0' && token[i] <= '9') ||
                                token[i] == '_'))
            {
                i++;
            }
            size_t var_len = i - var_start;
            char *var_name = malloc(var_len + 1);
            if (!var_name)
                break;
            strncpy(var_name, token + var_start, var_len);
            var_name[var_len] = '\0';
            char *env_value = check_env(env, var_name);
            free(var_name);
            if (env_value) {
                size_t k = 0;
                while (env_value[k]) {
                    result[j++] = env_value[k++];
                }
            }
        }
        else
            result[j++] = token[i++];
    }
    result[j] = '\0';
    return result;
}

void handle_echo(char **args, t_env *env)
{
    int i = 1;
    int line_flag = 0;
    
    if (args[1] && !ft_strcmp(args[1], "-n"))
	{
        line_flag = 1;
        i = 2;
    }
    while (args[i])
	{
        char *processed = special_check(args[i], env);
        if (processed)
		{
            printf("%s", processed);
            free(processed);
        }
        if (args[i + 1])
           printf(" ");
        i++;
    }
    if (!line_flag)
        printf("\n");
}

int		check_unclosed(char *line) // this still on work
{
	int		s_nbr;
	int		d_nbr;
	int		i;
	char	is_first = ' ';

	i = 0;
	s_nbr = 0;
	d_nbr = 0;
	while (line[i])
	{
		if (line[i] == '"' && is_first == ' ')
		{
			d_nbr++;
			is_first = line[i];
		}
		else if (line[i] == '\'' && is_first == ' ')
		{
			s_nbr++;
			is_first = line[i];
		}
		i++;
		if (line[i] == '"' && s_nbr % 2 != 0 && is_first == line[i])
		{
			printf("FATAL ERROR: Unclosed Single Quotes ... Try Again\n");
			return (1);
		}
		if (line[i] == '\'' && d_nbr % 2 != 0 && is_first == line[i])
		{
			printf("FATAL ERROR: Unclosed Double Quotes ... Try Again\n");
			return (1);
		}
	}
	return (0);
}

typedef struct s_dir
{
    char    *oldir;
    char    *dir;
    char    *home;
}           t_dir;

void    update_directory(t_dir *dir, t_env *my_env)
{
    t_env   *tmp;
    char    buf[PATH_MAX];

    if (!my_env)
        printf("FATAL ERROR: Environement Not Found ...\n");
    else
    {
        tmp = my_env;
        getcwd(buf, sizeof(buf));
        while (tmp)
        {
            if (!ft_strcmp(tmp->var, "PWD"))
                dir->dir = ft_strdup(tmp->value);
            if (!ft_strcmp(tmp->var, "OLDPWD"))
                dir->oldir = ft_strdup(tmp->value);
            if (!ft_strcmp(tmp->var, "HOME"))
                dir->home = ft_strdup(tmp->value);
            tmp = tmp->next;
        }
        if (!dir->dir)
            dir->dir = ft_strdup(buf);
        if (!dir->oldir)
            dir->oldir = ft_strdup(buf);
    }
}

char    *ft_getpath(t_env *env, char *str)
{
    t_env *tmp;
    char *str_key;
    char *str_value;

    tmp = env;
    str_value = str;
    while (tmp)
    {
        if (!ft_strcmp(tmp->var, str))
            return (tmp->value);
        tmp = tmp->next;
    }
    return ("/root");
}

void    handle_cd(char **split, t_env *my_env)
{
    char tmp[PATH_MAX];

    if (!split[1] || (split[1] && split[1] == "~"))
    {
        if (chdir(ft_getpath(my_env, "HOME")) == -1)
        {
            if (access(ft_getpath(my_env, "HOME"), W_OK | R_OK) == -1)
                printf("cd: no such file or directory\n");
            else if (access(ft_getpath(my_env, "HOME"), X_OK) == -1)
                printf("cd: permission denied\n");
        }
        getcwd(tmp, sizeof(tmp));
        // update the env

    }
}

int    check_existant(t_env *my_env, char *str)
{
    t_env   *tmp;
    char    *str_key;
    char    *str_value;

    tmp = my_env;
    str_key = ft_substr(str, 0, strchr(str, '=') - str);
    str_value = ft_strdup(strchr(str, '=') + 1);
    while (tmp)
    {
        if (!ft_strcmp(tmp->var, str_key))
        {
            if (tmp->value)
                free(tmp->value);
            tmp->value = ft_strdup(str_value);
            return (0);
        }
        tmp = tmp->next;
    }
    if (str_value)
        free(str_value);
    return(1);
}

int    main(int ac, char **av, char **env)
{
    char        *line;
	t_env		*my_env;
    t_dir       directory;
    char        **split;
	int			has_unclosed_quotes = 0;
    int         i;
    int         j;

    (void)ac;
    (void)av;
	line = NULL;
	split = NULL;
    directory.dir = NULL;
    directory.oldir = NULL;
    directory.home = NULL;

	my_env = create_env(env);
    update_directory(&directory, my_env);
    while (1)
    {
        line = readline("$> ");
        if (!line)
		{
            printf("exit\n");
            free(line);
			if (split)
            	free_array(split);
            break;
        }

        split = parse_prompt_to_argv(line);
		if (line[0] == '\0' || !split || !split[0])
		{
            free(line);
			if (split)
				free_array(split);
			split = NULL;
            continue;
        }

		// this to check for unclosed quotes
		i = 0;
		while (split[i])
        {
            if (check_unclosed(split[i]) == 1)
            {
                has_unclosed_quotes = 1;
                break;
            }
            i++;
        }
        if (has_unclosed_quotes)
        {
			add_history(line);
            free_array(split);
            free(line);
            continue;
        }

		//handle exit
        if (!ft_strcmp(split[0], "exit"))
		{
            printf("exit\n");
            free(line);
			free_array(split);
			split = NULL;
            break;
        }

		// handle echo
        if (!ft_strcmp(split[0], "echo"))
            handle_echo(split, my_env);

		// handle env
		if (!ft_strcmp(split[0], "env"))
			parse_env(my_env, 0);

		// handle unset
		if (!ft_strcmp(split[0], "unset"))
		{
			if (!split[1])
				continue;
			my_env = clear_node(my_env, split[1]);
		}

		// handle export
		else if (!ft_strcmp(split[0], "export"))
		{
			if (!split[1])
				parse_env(my_env, -1);
			else if (!strchr(split[1], '='))
            {
                add_history(line);
                free(line);
                free_array(split);
				continue;
            }
            if (check_existant(my_env, split[1]) == 1)
			    ft_lstadd_back(&my_env, create_node(split[1]));
		}

		// handle pwd
		if (!ft_strcmp(split[0], "pwd"))
            printf("%s\n", directory.dir);

        // handle cd
        // if (!ft_strcmp(split[0], "cd"))
        // {
        //     if (!split[1] || (split[1] && split[1] == '~'))
        //     {
        //         if (chdir(ft_getpath(my_env, "HOME") == -1))
        //         {
        //             if (access(ft_getpath(my_env, "HOME"), W_OK | R_OK) == -1)
        //                 printf("cd: no such file or directory\n");
        //             else if (access(ft_getpath(my_env, "HOME"), X_OK) == -1)
        //                 printf("cd: permission denied\n");
        //         }
        //         getcwd()
        //     }
        // }

        // handle clear

        add_history(line);
        free(line);
        if (split)
        {
            free_array(split);
            split = NULL;
        }
    }
    if (directory.dir)
    {
        free(directory.dir);
        directory.dir = NULL;
    }
    if (directory.oldir)
    {
        free(directory.oldir);
        directory.oldir = NULL;
    }
    if (directory.home)
    {
        free(directory.home);
        directory.home = NULL;
    }
	lst_clean(my_env);
    rl_clear_history();
    return (0);
}

// if sig of ctr+C, use rl_on_new_line, rl_replace_line and rl_redisplay