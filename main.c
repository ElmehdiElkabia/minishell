#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <dirent.h>
#include <linux/limits.h>
#include <limits.h>
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

typedef struct s_dir
{
    char    *oldir;
    char    *dir;
    char    *home;
    int     exit_status_;
}           t_dir;

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

int is_not_alpha_num_equal(char *str)
{
    int danger_area = 0;
    if (!*str)
        return (0);
    while (*str)
    {
        if (*str == '=')
            danger_area = 1;
        if (!((*str >= '0' && *str <= '9') ||
              (*str >= 'A' && *str <= 'Z') ||
              (*str >= 'a' && *str <= 'z')) &&
              !danger_area)
            return (1);
        str++;
    }
    return (0);
}

int is_num(char *str)
{
    if (!*str)
        return (0);
    while (*str)
    {
        if ((*str > '9' || *str < '0') && *str != '\"' && *str != '"')
            return (1);
        str++;
    }
    return (0);
}

int is_all_num(char *str)
{
    if (!*str)
        return (0);
    while (*str)
    {
        if ((*str >= 'A' && *str <= 'Z') ||
              (*str >= 'a' && *str <= 'z'))
            return (0);
        str++;
    }
    return (1);
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

static unsigned long long	ft_helper(const char *str, int *i
            , unsigned long long *number, int *quote_type)
{
	while (str[*i])
	{
		if (str[*i] >= '0' && str[*i] <= '9')
		{
			*number = (*number * 10) + (str[(*i)++] - '0');
			if (*number >= LONG_MAX)
				return (-1);
		}
		else if (str[*i] == '\"' || str[*i] == '"')
			{
                if (*quote_type == 0)
                    *quote_type = (str[*i] == '\"') ? 1 : 2;
                else if ((*quote_type == 1 && str[*i] == '\'') 
                        || (*quote_type == 2 && str[*i] == '\"'))
                {
                    printf("✘ mish: exit: %s: numeric argument required\n", str);
                    return (2);
                }
                (*i)++; // Skip quotes
            }
		else
			break;
	}
	return (*number);
}

int	ft_atoi(const char *str)
{
	unsigned long long	number;
	int					sign;
	int					i;
    int                 quote_flag;

	sign = 1;
	number = 0;
	i = 0;
    quote_flag = 0;
	if (!str)
		return (0);
	while (str[i] && (str[i] == ' ' || str[i] == '\t' || str[i] == '\n'
			|| str[i] == '\r' || str[i] == '\v' || str[i] == '\f'
            || str[i] == '\"' || str[i] == '"'))
    {
        if (str[i] == '\"' && !quote_flag)
            quote_flag = 1; // for ""
        else if (str[i] == '"' && !quote_flag)
            quote_flag = 2; // for ''
		i++;
    }
    if (str[i] == '\"' && !quote_flag)
            quote_flag = 1; // for ""
        else if (str[i] == '"' && !quote_flag)
            quote_flag = 2; // for ''
	if (str[i] == '-' || str[i] == '+')
	{
		if (str[i] == '-')
			sign = -1;
		i++;
	}
    if (str[i] == '\"' && !quote_flag)
            quote_flag = 1; // for ""
        else if (str[i] == '"' && !quote_flag)
            quote_flag = 2; // for ''
    while (str[i] && (str[i] == '\"' || str[i] == '"'))
    {
        if (str[i] == '\"' && !quote_flag)
            quote_flag = 1; // for ""
        else if (str[i] == '"' && !quote_flag)
            quote_flag = 2; // for ''
        i++;
    }
	number = ft_helper(str, &i, &number, &quote_flag);
	if ((int)number == -1 || (int)number == 0)
		return ((int)number);
	return (sign * number);
}

char	*ft_substr(char *s, int start, size_t len)
{
	size_t		i;
	char		*t;

	if (!s)
		return (NULL);
	if (start >= ft_strlen(s))
		return (ft_strdup(""));
	i = 0;
	if ((int)len > ft_strlen(s) - start)
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

void	parse_env(t_env *lst, int k, t_dir *dir)
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
    dir->exit_status_ = 0;
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

void int_to_str(int num, char *buffer)
{
    int i = 0;
    int is_negative = num < 0;

    if (is_negative)
        num = -num;
    if (num == 0)
        buffer[i++] = '0';
    while (num > 0)
    {
        buffer[i++] = (num % 10) + '0';
        num /= 10;
    }
    if (is_negative)
        buffer[i++] = '-';
    int start = 0, end = i - 1;
    while (start < end)
    {
        char temp = buffer[start];
        buffer[start] = buffer[end];
        buffer[end] = temp;
        start++;
        end--;
    }
    buffer[i] = '\0';
}

char *special_check(const char *token, t_env *env, int last_exit_status)
{
    char *result = malloc(PATH_MAX);
    if (!result)
        return NULL;

    size_t i = 0, j = 0;
    int in_double_quotes = 0;
    int in_single_quotes = 0;

    while (token[i])
    {
        if (token[i] == '"' && !in_single_quotes)
            in_double_quotes = !in_double_quotes;
        else if (token[i] == '\'' && !in_double_quotes)
            in_single_quotes = !in_single_quotes;
        if (token[i] == '$' && !in_single_quotes)
        {
            i++;
            if (token[i] == '?')
            {
                char exit_status[12];
                int_to_str(last_exit_status, exit_status);
                size_t k = 0;
                while (exit_status[k])
                    result[j++] = exit_status[k++];
                i++;
            }
            else if (token[i] == '0')
            {
                strcpy(result + j, "mish");
                j += strlen("mish");
                i++;
            }
            else if (!((token[i] >= 'A' && token[i] <= 'Z') ||
                     (token[i] >= 'a' && token[i] <= 'z') ||
                     (token[i] >= '0' && token[i] <= '9') ||
                     token[i] == '_'))
            {
                while(token[i] == '"' || token[i] == '\'')
                    i++;
            }
            else if ((token[i] >= 'A' && token[i] <= 'Z') ||
                     (token[i] >= 'a' && token[i] <= 'z') ||
                     (token[i] >= '0' && token[i] <= '9') ||
                     token[i] == '_')
            {
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
                if (env_value)
                {
                    size_t k = 0;
                    while (env_value[k])
                        result[j++] = env_value[k++];
                }
            }
            else
                result[j++] = '$';
        }
        else
        {
            result[j++] = token[i++];
        }
    }
    result[j] = '\0';
    return result;
}

void handle_echo(char **args, t_env *env, t_dir *dir)
{
    int i = 1;
    int line_flag = 0;
    int len;
    int j;
    char quotes;

    if (args[1] && !ft_strcmp(args[1], "-n"))
    {
        line_flag = 1;
        i = 2;
    }
    while (args[i])
    {
        j = 0;
        if (!args[i][0])
        {
            i++;
            continue;
        }
        len = ft_strlen(args[i]);
        if (args[i][0] == '"' || args[i][0] == '\'')
        {
            j++;
            len--;
            quotes = args[i][0];
        }
        if (len > 0 && args[i][len - 1] == quotes)
            len--;
        while (j < len)
        {
            write(1, &args[i][j], 1);
            j++;
        }
        if (args[i + 1])
            write(1, " ", 1);
        i++;
    }
    if (!line_flag)
        write(1, "\n", 1);
    dir->exit_status_ = 0;
}

int check_unclosed(char *line)
{
    int i = 0;
    int in_single_quote = 0;
    int in_double_quote = 0;

    while (line[i])
    {
        if (line[i] == '\'' && !in_double_quote)
            in_single_quote = !in_single_quote;
        if (line[i] == '"' && !in_single_quote)
            in_double_quote = !in_double_quote;
        i++;
    }
    if (in_single_quote)
    {
        printf("✘ FATAL ERROR: Unclosed Single Quotes ... Try Again\n");
        return 1;
    }
    if (in_double_quote)
    {
        printf("✘ FATAL ERROR: Unclosed Double Quotes ... Try Again\n");
        return 1;
    }
    return 0;
}

void    update_directory(t_dir *dir, t_env *my_env)
{
    t_env   *tmp;
    char    buf[PATH_MAX];

    if (!my_env)
    {
        printf("✘ FATAL ERROR: Environement Not Found ...\n");
        dir->exit_status_ = 1;
    }
    else
    {
        tmp = my_env;
        if (!getcwd(buf, sizeof(buf)))
        {
            perror("✘ getcwd");
            return;
        }
        while (tmp)
        {
            if (!ft_strcmp(tmp->var, "PWD"))
            {
                if (dir->dir)
                    free(dir->dir);
                dir->dir = ft_strdup(tmp->value);
            }
            if (!ft_strcmp(tmp->var, "OLDPWD"))
            {
                if (dir->oldir)
                    free(dir->oldir);
                dir->oldir = ft_strdup(tmp->value);
            }
            if (!ft_strcmp(tmp->var, "HOME"))
            {
                if (dir->home)
                    free(dir->home);
                dir->home = ft_strdup(tmp->value);
            }
            tmp = tmp->next;
        }
        if (!dir->dir)
            dir->dir = ft_strdup(buf);
        if (!dir->oldir)
            dir->oldir = ft_strdup(buf);
    }
}

void    handle_cd(char **split, t_env *my_env, t_dir *dir)
{
    char    buf[PATH_MAX];
    char   *smp;
    t_env   *tmp;

    tmp = my_env;
    smp = NULL;
    if (!split[1] || (split[1] && split[1][0] == '~'))
    {
        if (chdir(dir->home) == -1)
        {
            if ((access(dir->home, W_OK | R_OK) == -1) || (access(dir->home, X_OK) == -1))
            {
                perror("✘ cd");
                dir->exit_status_ = 1;
            }
            return ;
        }
        if (!getcwd(buf, sizeof(buf)))
        {
            perror("✘ getcwd");
            return;
        }
        while (tmp)
        {
            if (!ft_strcmp(tmp->var, "PWD"))
            {
                smp = ft_strdup(tmp->value);
                if (tmp->value)
                    free (tmp->value);
                tmp->value = ft_strdup(buf);
            }
            if (!ft_strcmp(tmp->var, "OLDPWD"))
            {
                if (tmp->value)
                    free (tmp->value);
                tmp->value = ft_strdup(smp);
                break;
            }
            tmp = tmp->next;
        }
    }
    else if (!ft_strcmp(split[1], "-"))
    {
        if (chdir(dir->oldir) == -1)
        {
            if ((access(dir->oldir, W_OK | R_OK) == -1) || (access(dir->oldir, X_OK) == -1))
            {
                perror("✘ cd");
                dir->exit_status_ = 1;
            }
            return ;
        }
        if (!getcwd(buf, sizeof(buf)))
        {
            perror("✘ getcwd");
            return;
        }
        while (tmp)
        {
            if (!ft_strcmp(tmp->var, "PWD"))
            {
                smp = ft_strdup(tmp->value);
                if (tmp->value)
                    free (tmp->value);
                tmp->value = ft_strdup(buf);
            }
            if (!ft_strcmp(tmp->var, "OLDPWD"))
            {
                if (tmp->value)
                    free (tmp->value);
                tmp->value = ft_strdup(smp);
                break;
            }
            tmp = tmp->next;
        }
        printf("%s\n", buf);
    }
    else if (ft_strcmp(split[1], "-") && split[1][0] == '-')
        printf("✘ mish: cd: %s: invalid option\ncd: usage: cd [-L|[-P [-e]] [-@]] [dir]\n", split[1]);
    else if (!split[2])
    {
        if (chdir(split[1]) == -1)
        {
            if ((access(split[1], W_OK | R_OK) == -1) || (access(split[1], X_OK) == -1))
            {
                perror("✘ cd");
                dir->exit_status_ = 1;
            }
            return ;
        }
        if (!getcwd(buf, sizeof(buf)))
        {
            perror("✘ getcwd");
            return;
        }
        while (tmp)
        {
            if (!ft_strcmp(tmp->var, "PWD"))
            {
                smp = ft_strdup(tmp->value);
                if (tmp->value)
                    free (tmp->value);
                tmp->value = ft_strdup(buf);
            }
            if (!ft_strcmp(tmp->var, "OLDPWD"))
            {
                if (tmp->value)
                    free (tmp->value);
                tmp->value = ft_strdup(smp);
                break;
            }
            tmp = tmp->next;
        }
    }
    else
    {
        write(1, "✘ mish: cd: too many arguments\n", 33);
        dir->exit_status_ = 1;
    }
    update_directory(dir, my_env);
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

void    tilda_remod(char *line, t_dir dir)
{
    int     i;
    char    *tmp;

    i = 0;
    while (line[i])
    {
        if (line[0] == '~' && (!line[1] || line[1] == '/'))
        {
            tmp = malloc(strlen(dir.home) + strlen(line) + 1);
            if (!tmp)
            {
                perror("malloc");
                return ;
            }
            strcpy(tmp, dir.home);
            strcat(tmp, line + 1);
            free(line);
            line = ft_strdup(tmp);
            free(tmp);
        }
        else if (line[0] == '~' && line[1] != '/')
        {
            tmp = malloc(strlen("/home/") + strlen(line) + 1);
            strcpy(tmp, "/home/");
            strcat(tmp, line + 1);
            if (access(tmp, F_OK) == 0)
            {
                free(line);
                line = ft_strdup(tmp);
                free(tmp);
            }
        }
        i++;
    }
}

int nested_quotes(char *str)
{
    int i = 0;
    int double_quotes = 0;
    int single_quotes = 0;

    if (!*str)
        return (0);
    while (str[i])
    {
        if (str[i] == '"')
            double_quotes = 1;
        else if (str[i] == '\'')
            single_quotes = 1;
        i++;
    }
    if (double_quotes && single_quotes)
        return (2);
    return (0);
}

void    handle_exit(char *line, char **split, t_dir *directory)
{
    directory->exit_status_ = 0;
    if (split[2])
    {
        write(1, "exit\n✘ mish: exit: too many arguments\n", 41);
        directory->exit_status_ = 1;
    }
    else if ((is_num(split[1]) && !ft_atoi(split[1])) || (nested_quotes(split[1]) == 2))
    {
        printf("exit\n✘ mish: exit: %s: numeric argument required\n", split[1]);
        directory->exit_status_ = 2;
    }
    else
    {
        printf("exit\n");
        if (split[1])
            directory->exit_status_ = ft_atoi(split[1]);
    }
    free(line);
    free_array(split);
    split = NULL;
}

void    handle_export(t_dir *directory, char **split, t_env *my_env)
{
    directory->exit_status_ = 0;
    if (!split[1])
        parse_env(my_env, -1, directory);
    else if ((directory->exit_status_ = is_not_alpha_num_equal(split[1])) 
            || (directory->exit_status_ = is_all_num(split[1])))
    {
        printf("✘ mish: export: `%s' : not a valid identifier\n", split[1]);
        directory->exit_status_ = 1;
    }
    else if (!strchr(split[1], '='))
        return;
    else if (check_existant(my_env, split[1]) == 1)
        ft_lstadd_back(&my_env, create_node(split[1]));
}

int     check_meta_char(char *str)
{
    int i;

    i = 0;
    char *unsupported = "^&*()+@!;\\{}[]~`";
    while (str[i])
    {
        if (strchr(unsupported, str[i]))
        {
            printf("✘ mish: Unsupported character: %c\n", str[i]);
            return (1);
        }
        i++;
    }
    return (0);
}

int    main(int ac, char **av, char **env)
{
    char        *line;
	t_env		*my_env;
    t_dir       directory;
    char        **split;
    int         i;

    (void)ac;
    (void)av;
	line = NULL;
	split = NULL;
    directory.dir = NULL;
    directory.oldir = NULL;
    directory.home = NULL;
    directory.exit_status_ = 0;

	my_env = create_env(env);
    update_directory(&directory, my_env);
    while (1)
    {
        line = readline("✧ mi/sh ➤ ");
        if (!line)
		{
            printf("exit\n");
            free(line);
            directory.exit_status_ = 0;
            break;
        }
        add_history(line);
        if (check_unclosed(line) || check_meta_char(line))
        {
            directory.exit_status_ = 1;
            continue;
        }
        char *expanded = special_check(line, my_env, directory.exit_status_);
        if (expanded)
        {
            free(line);
            line = ft_strdup(expanded);
            free(expanded);
        }
        
        // handle the glorious tilde
        tilda_remod(line, directory);        
        
        split = parse_prompt_to_argv(line);
        if (line[0] == '\0' || !split || !split[0])
        {
            if (split)
                free_array(split);
            split = NULL;
            directory.exit_status_ = 0;
            continue;
        }
        else if (!ft_strcmp(split[0], "exit"))
		{
            handle_exit(line, split, &directory);
            break;
        }
        else if (!ft_strcmp(split[0], "echo"))
            handle_echo(split, my_env, &directory);
		else if (!ft_strcmp(split[0], "env"))
			parse_env(my_env, 0, &directory);
		else if (!ft_strcmp(split[0], "unset"))
		{
            directory.exit_status_ = 0;
			if (!split[1])
			{
                add_history(line);
                free(line);
                free_array(split);
				continue;
            }
			my_env = clear_node(my_env, split[1]);
		}
		else if (!ft_strcmp(split[0], "export"))
            handle_export(&directory, split, my_env);
		else if (!ft_strcmp(split[0], "pwd"))
        {
            printf("%s\n", directory.dir);
            directory.exit_status_ = 0;
        }
        else if (!ft_strcmp(split[0], "cd"))
            handle_cd(split, my_env, &directory);
        // handle clear
        else if (!ft_strcmp(split[0], "clear"))
            write(1, "\033[2J\033[H", 8);
            
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
    directory.exit_status_ %= 256;
    directory.exit_status_ = (directory.exit_status_ < 0) ? 256 + directory.exit_status_ : directory.exit_status_;
    return (directory.exit_status_);
}

// if sig of ctr+C, use rl_on_new_line, rl_replace_line and rl_redisplay