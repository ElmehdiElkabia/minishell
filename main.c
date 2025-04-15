#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <dirent.h>
#include <linux/limits.h>
#include <limits.h>
#include <signal.h>
#include <fcntl.h>
#include <readline/readline.h>
#include <readline/history.h>

// echo (-n), cd, pwd, export, unset, env, exit, clear

// printf must be remplaced by ft_printf

// must check if file in is_builtins

volatile sig_atomic_t signal_global;

typedef enum e_role
{
	CMD,	  // Regular command
	REDIR_IN, //  <
	HDOC,	  //  <<
	TRUNC,	  //  >
	APPEND	  //  >>
} t_role;

typedef enum e_stat
{
	LOOP = 0,
	NO_LOOP = 1
} t_stat;

typedef struct s_cmd
{
	char *cmd;
	t_role role;
	struct s_cmd *next;
} t_cmd;

typedef struct s_env
{
	char *var;
	char *value;
	struct s_env *next;
} t_env;

typedef struct s_dir
{
	char *oldir;
	char *dir;
	char *home;
	long exit_status_;
} t_dir;

typedef struct s_signal
{
	struct sigaction sa_int;
	struct sigaction sa_quit;
} t_signal;

int ft_isspace(int c)
{
	return (c == ' ' || c == '\t' || c == '\n' ||
			c == '\r' || c == '\v' || c == '\f');
}

size_t ft_strlcpy(char *dst, const char *src, size_t dstsize)
{
	size_t i;
	size_t j;

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

static void iclean(char **t, int end)
{
	int i;

	i = 0;
	while (i < end)
	{
		free(t[i]);
		i++;
	}
	free(t);
}

static int count_words(char const *s, char c)
{
	int i;
	int k;
	int cpt;

	i = 0;
	cpt = 0;
	k = 1;
	while (s[i])
	{
		if (s[i] == c)
			k = 1;
		else if (s[i] != c && k == 1)
		{
			cpt++;
			k = 0;
		}
		i++;
	}
	return (cpt);
}

static char *cl_al_fi(char const *s, char c)
{
	int i;
	int cpt;
	char *p;

	i = 0;
	cpt = 0;
	while (s[i] && s[i] == c)
		i++;
	while (s[i + cpt] && s[i + cpt] != c)
		cpt++;
	p = malloc((cpt + 1) * sizeof(char));
	if (p)
	{
		ft_strlcpy(p, s + i, cpt + 1);
		p[cpt] = '\0';
	}
	return (p);
}

char **sub_split(char **strs, char c, char const *s)
{
	int i;

	i = 0;
	while (*s)
	{
		while (*s == c)
			s++;
		if (*s)
		{
			strs[i] = cl_al_fi(s, c);
			if (!strs[i])
			{
				iclean(strs, i);
				return (NULL);
			}
			i++;
			while (*s && *s != c)
				s++;
		}
	}
	strs[i] = NULL;
	return (strs);
}

char **ft_split(char const *s, char c)
{
	char **strs;

	if (!s)
		return (NULL);
	strs = malloc((count_words(s, c) + 1) * sizeof(char *));
	if (!strs)
		return (NULL);
	strs = sub_split(strs, c, s);
	return (strs);
}

void lst_clean(t_env *head)
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

void free_array(char **arr)
{
	int i;

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

int ft_strlen(const char *s)
{
	int i;

	if (!s)
		return (0);
	i = 0;
	while (s[i])
		i++;
	return (i);
}

char *ft_strdup(const char *s1)
{
	char *t1;
	int i;

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

int ft_strcmp(char *str1, const char *str2)
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

void error_par(char **data, const char *msg)
{
	write(2, msg, ft_strlen(msg));
	if (data)
		free_array(data);
	exit(1);
}

void ft_lstadd_back(t_env **lst, t_env *new)
{
	t_env *temp;

	if (!lst)
		return;
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

static unsigned long long ft_helper(const char *str, int *i, unsigned long long *number, int *quote_type)
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
			else if ((*quote_type == 1 && str[*i] == '\'') || (*quote_type == 2 && str[*i] == '\"'))
			{
				printf("✘ mish: exit: %s: numeric argument required\n", str);
				return (2);
			}
			(*i)++;
		}
		else
			break;
	}
	return (*number);
}

unsigned long long ft_atoi(const char *str)
{
	unsigned long long number;
	int sign;
	int i;
	int quote_flag;

	sign = 1;
	number = 0;
	i = 0;
	quote_flag = 0;
	if (!str)
		return (0);
	while (str[i] && (str[i] == ' ' || str[i] == '\t' || str[i] == '\n' || str[i] == '\r' || str[i] == '\v' || str[i] == '\f' || str[i] == '\"' || str[i] == '"'))
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
		return (number);
	return (sign * number);
}

void *ft_realloc(void *ptr, size_t old_size, size_t new_size)
{
	if (new_size == 0)
	{
		free(ptr);
		return NULL;
	}

	void *new_ptr = malloc(new_size);
	if (!new_ptr)
	{
		perror("malloc");
		return NULL;
	}

	if (ptr)
	{
		size_t copy_size = old_size < new_size ? old_size : new_size;
		memcpy(new_ptr, ptr, copy_size); //
		free(ptr);
	}
	return new_ptr;
}

char *ft_substr(char *s, int start, size_t len)
{
	size_t i;
	char *t;

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
	t_env *new_node;

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
	int i;
	int flag;
	t_env *lst;
	t_env *current;
	t_env *new_node;

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

void parse_env(t_env *lst, int k, t_dir *dir)
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
	int is_first = 0; // 1 for single and 2 for double

	while (token[i])
	{
		if (token[i] == '"' && !in_single_quotes)
		{
			in_double_quotes = !in_double_quotes;
			if (!is_first)
				is_first = 2;
			i++;
		}
		else if (token[i] == '\'' && !in_double_quotes)
		{
			in_single_quotes = !in_single_quotes;
			if (!is_first)
				is_first = 1;
			i++;
		}
		else if ((token[i] == '"' && !in_single_quotes) || (token[i] == '\'' && !in_double_quotes))
			result[j++] = token[i++];
		else if (token[i] == '$' && !token[i + 1])
			result[j++] = token[i++];
		else if (token[i] == '$' && !in_single_quotes)
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
				while (token[i] == '"' || token[i] == '\'')
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
		else if ((is_first == 2 && token[i] == '"') || (is_first == 1 && token[i] == '\''))
			i++;
		else
			result[j++] = token[i++];
	}
	result[j] = '\0';
	return result;
}

int is_all_n(char *str)
{
	int i;

	i = 0;
	if (str[0] != '-')
		return (1);
	i++;
	while (str[i])
	{
		if (str[i] != 'n')
			return (1);
		i++;
	}
	return (0);
}

void handle_echo(char **args, t_env *env, t_dir *dir)
{
	int i = 1;
	int line_flag = 0;
	int len;
	int j;
	char quotes;

	if (args[1] && (!ft_strcmp(args[1], "-n") || !is_all_n(args[1])))
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

void update_directory(t_dir *dir, t_env *my_env)
{
	t_env *tmp;
	char buf[PATH_MAX];

	if (!my_env)
	{
		printf("✘ mish: env: Environement Not Found ...\n");
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

void handle_cd(char **split, t_env *my_env, t_dir *dir)
{
	char buf[PATH_MAX];
	char *smp;
	t_env *tmp;

	tmp = my_env;
	smp = NULL;
	if (!split[1])
	{
		if (chdir(dir->home) == -1)
		{
			if ((access(dir->home, W_OK | R_OK) == -1) || (access(dir->home, X_OK) == -1))
			{
				perror("✘ cd");
				dir->exit_status_ = 1;
			}
			return;
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
					free(tmp->value);
				tmp->value = ft_strdup(buf);
			}
			if (!ft_strcmp(tmp->var, "OLDPWD"))
			{
				if (tmp->value)
					free(tmp->value);
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
			return;
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
					free(tmp->value);
				tmp->value = ft_strdup(buf);
			}
			if (!ft_strcmp(tmp->var, "OLDPWD"))
			{
				if (tmp->value)
					free(tmp->value);
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
			return;
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
					free(tmp->value);
				tmp->value = ft_strdup(buf);
			}
			if (!ft_strcmp(tmp->var, "OLDPWD"))
			{
				if (tmp->value)
					free(tmp->value);
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

int check_existant(t_env *my_env, char *str)
{
	t_env *tmp;
	char *str_key;
	char *str_value;

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
	return (1);
}

// void    tilda_remod(char *line, t_dir dir)
// {
//     int i = 0;
//     if (!line || !strchr(line, '~'))
//         //return (strdup(line));
//         return;

//     char *tmp;
//     char *new;
//     while (line[i])
//     {
//         tmp = strchr(line, '~');
//         if (!tmp || (tmp[1]))
//         new = malloc(strlen(line) + strlen(dir.home));
//         strncpy(new, line, ft_strlen(line)-ft_strlen(tmp));
//         strcat(new, dir.home);
//         strcat(new, tmp + 1);
//         printf("%s\n", new);
//         i++;
//     }
// }

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

void handle_exit(char *line, char **split, t_dir *directory, t_stat *STATUS)
{
	*STATUS = 1;
	directory->exit_status_ = 0;
	if (split[1] && split[2])
	{
		write(1, "exit\n✘ mish: exit: too many arguments\n", 41);
		directory->exit_status_ = 1;
		*STATUS = 0;
	}
	else if (split[1] && ((is_num(split[1]) && !ft_atoi(split[1])) || (nested_quotes(split[1]) == 2)))
	{
		printf("exit\n✘ mish: exit: %s: numeric argument required\n", split[1]);
		directory->exit_status_ = 2;
	}
	else if (split[1] && ((split[1][0] != '-' &&
						   (ft_strlen(split[1]) > 19 ||
							(ft_strlen(split[1]) == 19 && strcmp(split[1], "9223372036854775807") > 0))) ||
						  (split[1][0] == '-' &&
						   (ft_strlen(split[1]) > 20 ||
							(ft_strlen(split[1]) == 20 && strcmp(split[1], "-9223372036854775808") > 0)))))
	{
		printf("exit\n✘ mish: exit: %s: numeric argument required\n", split[1]);
		directory->exit_status_ = 2;
	}
	else
	{
		printf("exit\n");
		if (split[1])
			directory->exit_status_ = ft_atoi(split[1]);
		if (split[1] && (ft_strlen(split[1]) == 20 && !ft_strcmp(split[1], "-9223372036854775808")))
			directory->exit_status_ = 0;
	}
}

void handle_export(t_dir *directory, char **split, t_env *my_env)
{
	directory->exit_status_ = 0;
	if (!split[1])
		parse_env(my_env, -1, directory);
	else if ((directory->exit_status_ = is_not_alpha_num_equal(split[1])) || (directory->exit_status_ = is_all_num(split[1])))
	{
		printf("✘ mish: export: `%s' : not a valid identifier\n", split[1]);
		directory->exit_status_ = 1;
	}
	else if (!strchr(split[1], '='))
		return;
	else if (check_existant(my_env, split[1]) == 1)
		ft_lstadd_back(&my_env, create_node(split[1]));
}

int check_meta_char(char *str)
{
	int i;

	i = 0;
	char *unsupported = "!#%%&()*,:;@[\\]^`{}";
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

char *expand_user(const char *user)
{
	char path[256];
	strcpy(path, "/home/");
	strcat(path, user);
	if (access(path, F_OK) == 0)
		return ft_strdup(path);
	return NULL;
}

char *expand_tilde(const char *token, t_dir dir)
{
	if (token[0] == '~')
	{
		if (token[1] == '/' || token[1] == '\0')
		{
			char *result = malloc(strlen(dir.home) + strlen(token) + 1);
			if (!result)
			{
				perror("malloc");
				return NULL;
			}
			strcpy(result, dir.home);
			if (token[1] == '/')
				strcat(result, token + 1);
			return result;
		}
		else
		{
			char *user = ft_strdup(token + 1);
			char *user_home = expand_user(user);
			free(user);
			return user_home;
		}
	}
	return ft_strdup(token);
}

void tilda_remod(char **line, t_dir dir)
{
	if (!*line || !dir.home)
		return;
	char **tokens = ft_split(*line, ' ');
	if (!tokens)
		return;
	char *new_line = malloc(1);
	if (!new_line)
	{
		perror("malloc");
		free_array(tokens);
		return;
	}
	new_line[0] = '\0';
	for (int i = 0; tokens[i]; i++)
	{
		char *expanded = expand_tilde(tokens[i], dir);
		if (expanded)
		{
			new_line = ft_realloc(new_line, strlen(new_line) + 1, strlen(new_line) + strlen(expanded) + 2);
			if (!new_line)
			{
				free(expanded);
				free_array(tokens);
				return;
			}
			strcat(new_line, expanded);
			strcat(new_line, " ");
			free(expanded);
		}
		else
		{
			new_line = ft_realloc(new_line, strlen(new_line) + 1, strlen(new_line) + strlen(tokens[i]) + 2);
			if (!new_line)
			{
				free_array(tokens);
				return;
			}
			strcat(new_line, tokens[i]);
			strcat(new_line, " ");
		}
	}
	if (strlen(new_line) > 0)
		new_line[strlen(new_line) - 1] = '\0';
	free(*line);
	*line = new_line;
	free_array(tokens);
}

void handle_builtins(char *line, char **split, t_dir *directory, t_stat *STATUS, t_env *my_env)
{
	if (!(line[0] == '\0' || !split || !split[0]))
	{
		if (!ft_strcmp(split[0], "exit"))
			handle_exit(line, split, directory, STATUS);
		else if (!ft_strcmp(split[0], "echo"))
			handle_echo(split, my_env, directory);
		else if (!ft_strcmp(split[0], "env"))
			parse_env(my_env, 0, directory);
		else if (!ft_strcmp(split[0], "unset"))
		{
			directory->exit_status_ = 0;
			if (split[1])
				my_env = clear_node(my_env, split[1]);
		}
		else if (!ft_strcmp(split[0], "export"))
			handle_export(directory, split, my_env);
		else if (!ft_strcmp(split[0], "pwd"))
		{
			printf("%s\n", directory->dir);
			directory->exit_status_ = 0;
		}
		else if (!ft_strcmp(split[0], "clear"))
			write(1, "\033[2J\033[H", 8);
	}
}

t_cmd *create_cmd_node(char *cmd, int role)
{
	t_cmd *new_node = malloc(sizeof(t_cmd));
	if (!new_node)
	{
		perror("malloc");
		return NULL;
	}
	new_node->cmd = ft_strdup(cmd);
	new_node->role = role;
	new_node->next = NULL;
	return new_node;
}

void add_cmd_to_list(t_cmd **head, t_cmd *new_node)
{
	if (!head || !new_node)
		return;
	if (!*head)
		*head = new_node;
	else
	{
		t_cmd *current = *head;
		while (current->next)
			current = current->next;
		current->next = new_node;
	}
}

char *trim_whitespace(char *str)
{
	char *end;
	while (isspace((unsigned char)*str))
		str++;
	end = str + strlen(str) - 1;
	while (end > str && isspace((unsigned char)*end))
		end--;
	*(end + 1) = '\0';
	return str;
}

// is directory
int is_dir(char *line, t_dir *dir)
{
	if (!access(line, X_OK))
	{
		printf("✘ mish: %s: Is a directory\n", line);
		dir->exit_status_ = 126;
		return (0);
	}
	return (1);
}

static void handle_sigint(int sig)
{
	(void)sig;
	signal_global = SIGINT;
	write(STDOUT_FILENO, "\n", 1);
	rl_replace_line("", 0);
	rl_on_new_line();
	rl_redisplay();
}

static void handle_sigquit(int sig)
{
	(void)sig;
	signal_global = SIGQUIT;
}

void setup_signals(t_signal *config)
{
	config->sa_int.sa_handler = handle_sigint;
	sigemptyset(&config->sa_int.sa_mask);
	config->sa_int.sa_flags = 0;
	sigaction(SIGINT, &config->sa_int, NULL);

	config->sa_quit.sa_handler = handle_sigquit;
	sigemptyset(&config->sa_quit.sa_mask);
	config->sa_quit.sa_flags = 0;
	sigaction(SIGQUIT, &config->sa_quit, NULL);
}

void ft_putstr_fd(char *s, int fd)
{
	int i;

	if (!s)
		return;
	i = 0;
	while (s[i])
	{
		write(fd, &s[i], 1);
		i++;
	}
}

void ft_putendl_fd(char *s, int fd)
{
	int i;

	if (!s)
		return;
	i = 0;
	while (s[i])
	{
		write(fd, &s[i], 1);
		i++;
	}
	write(fd, "\n", 1);
}

char *ft_strjoin(char const *s1, char const *s2)
{
	char *str;
	size_t i;
	size_t j;

	if (!s1 || !s2)
		return (NULL);
	i = 0;
	j = 0;
	str = (char *)malloc((ft_strlen(s1) + ft_strlen(s2) + 1) * sizeof(char));
	if (!str)
		return (NULL);
	while (s1[i])
		str[j++] = s1[i++];
	i = 0;
	while (s2[i])
		str[j++] = s2[i++];
	str[j] = '\0';
	return (str);
}

char *ft_strchr(const char *s, int c)
{
	size_t i;

	if (!s)
		return (NULL);
	i = 0;
	while (s[i])
	{
		if (s[i] == (char)c)
			return ((char *)s + i);
		i++;
	}
	if ((char)c == '\0')
		return ((char *)s + i);
	return (0);
}

int ft_strncmp(const char *s1, const char *s2, size_t n)
{
	size_t i;

	i = 0;
	while ((s1[i] || s2[i]) && i < n)
	{
		if ((unsigned char)s1[i] != (unsigned char)s2[i])
			return ((unsigned char)s1[i] - (unsigned char)s2[i]);
		i++;
	}
	return (0);
}

int is_builtin(char **args)
{
	if (!args[0])
		return 0;
	if (!ft_strncmp(args[0], "env", 4))
		return 1;
	if (!ft_strncmp(args[0], "pwd", 4))
		return 1;
	if (!ft_strncmp(args[0], "cd", 3))
		return 1;
	if (!ft_strncmp(args[0], "export", 7))
		return 1;
	if (!ft_strncmp(args[0], "unset", 6))
		return 1;
	if (!ft_strncmp(args[0], "echo", 5))
		return 1;
	if (!ft_strncmp(args[0], "exit", 5))
		return 1;
	return 0;
}

char *find_value_env(char *key, t_env **head)
{
	t_env *temp;

	temp = *head;
	while (temp != NULL)
	{
		if (ft_strlen(key) == ft_strlen(temp->var) && ft_strncmp(key, temp->var, ft_strlen(key)) == 0)
			return temp->value;
		temp = temp->next;
	}
	return (NULL);
}

void ft_error(char *str, int n)
{
	ft_putstr_fd(str, 2);
	exit(n);
}

void ft_perror(char *str, int n)
{
	perror(str);
	exit(n);
}

void ft_cmd(char *str, char **cmd, int n)
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

char *get_full_path(t_env *head)
{
	return find_value_env("PATH", &head);
}
char *path_join(char *cmd, char *part_path)
{
	char *path;
	char *tmp;

	tmp = ft_strjoin(part_path, "/");
	if (!tmp)
		return (NULL);
	path = ft_strjoin(tmp, cmd);
	free(tmp);
	return (path);
}

char *get_path(char *cmd, t_env *env)
{
	char **path_argv;
	char *path_dir;
	char *path;
	int i;

	path_dir = get_full_path(env);
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

char **convert_env(t_env *env)
{
	int count = 0;
	t_env *tmp = env;
	char **env_arr;

	while (tmp)
	{
		count++;
		tmp = tmp->next;
	}
	env_arr = malloc((count + 1) * sizeof(char *));
	if (!env_arr)
		return (NULL);
	tmp = env;
	for (int i = 0; i < count; i++)
	{
		env_arr[i] = malloc(strlen(tmp->var) + strlen(tmp->value) + 2);
		if (!env_arr[i])
		{
			return (NULL);
		}
		sprintf(env_arr[i], "%s=%s", tmp->var, tmp->value);
		tmp = tmp->next;
	}
	env_arr[count] = NULL;
	return (env_arr);
}

void ft_hundel(char **cmd, t_env *env)
{
	char **env_arr = convert_env(env);
	if (!env_arr)
		ft_perror("Memory allocation failed", 1);

	if (ft_strchr(cmd[0], '/') != NULL)
	{
		if (execve(cmd[0], cmd, env_arr) == -1)
		{
			ft_perror("Command execution failed", 126);
		}
	}
}

void execute_command(char *cmd, t_env *head, t_dir *dir, t_stat *STATUS)
{
	char **cmd_argv = ft_split(cmd, ' ');
	if (!cmd_argv)
	{
		dir->exit_status_ = 1; // Memory error status
		return;
	}

	if (is_builtin(cmd_argv))
	{
		handle_builtins(cmd, cmd_argv, dir, STATUS, head);
		free_array(cmd_argv);
		return;
	}

	if (!cmd_argv[0])
	{
		dir->exit_status_ = 127; // Command not found
		free_array(cmd_argv);
		return;
	}

	ft_hundel(cmd_argv, head);
	char *path = get_path(cmd_argv[0], head);
	if (!path)
	{
		ft_cmd("Command not found: ", cmd_argv, 127);
		dir->exit_status_ = 127;
		free_array(cmd_argv);
		return;
	}

	char **env_arr = convert_env(head);
	if (!env_arr)
	{
		free(path);
		free_array(cmd_argv);
		dir->exit_status_ = 1;
		return;
	}

	if (execve(path, cmd_argv, env_arr) == -1)
	{
		free(path);
		free_array(env_arr);
		ft_cmd("Execution failed: ", cmd_argv, 126);
		dir->exit_status_ = 126;
	}
	free_array(cmd_argv);
}

void check_redirections(char *str)
{
	int i;
	int fd;
	char **token;

	token = ft_split(str, ' ');
	i = 0;
	while (token[i])
	{
		if (strncmp(token[i], ">", 1) == 0 && token[i][1] == '\0' && token[i + 1])
		{
			fd = open(token[i + 1], O_CREAT | O_WRONLY | O_TRUNC, 0644);
			if (fd < 0)
				perror("open");
			else
				dup2(fd, STDOUT_FILENO);
		}
		else if (strncmp(token[i], ">>", 2) == 0 && token[i][2] == '\0' && token[i + 1])
		{
			fd = open(token[i + 1], O_CREAT | O_WRONLY | O_APPEND, 0644);
			if (fd < 0)
				perror("open");
			else
				dup2(fd, STDOUT_FILENO);
		}
		else if (strncmp(token[i], "<", 1) == 0 && token[i][1] == '\0' && token[i + 1])
		{
			fd = open(token[i + 1], O_RDONLY);
			if (fd < 0)
				perror("open");
			else
				dup2(fd, STDIN_FILENO);
		}
		else if (strncmp(token[i], "<<", 2) == 0 && token[i][2] == '\0' && token[i + 1])
		{
			int fd_pipe[2];
			char *line;
			char *delim = token[i + 1];

			if (pipe(fd_pipe) < 0)
				perror("pipe");

			while (1)
			{
				line = readline("> ");
				if (!line)
				{
					close(fd_pipe[1]);
					break;
				}
				if (strncmp(line, delim, strlen(delim)) == 0 && line[strlen(delim)] == '\0')
				{
					free(line);
					break;
				}
				write(fd_pipe[1], line, strlen(line));
				write(fd_pipe[1], "\n", 1);
				free(line);
			}
			close(fd_pipe[1]);
			dup2(fd_pipe[0], STDIN_FILENO);
			close(fd_pipe[0]);
		}

		i++;
	}
}

char *strip_redirections(char *cmd)
{
	// printf("cmd ==> %s\n", cmd);
	char **tokens = ft_split(cmd, ' ');
	char *new_cmd = NULL;
	char *tmp;
	int i = 0;

	while (tokens[i])
	{
		// printf("tokens[%d] ==> %s\n", i, tokens[i]);
		if (strcmp(tokens[i], ">") == 0 || strcmp(tokens[i], "<") == 0 || strcmp(tokens[i], ">>") == 0 || strcmp(tokens[i], "<<") == 0)
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

typedef struct s_command
{
	char **args;   // Command + arguments (e.g., ["ls", "-l", NULL])
	char *infile;  // Input redirection file (<)
	char *outfile; // Output redirection file (>)
	int append;	   // 1 if >> (append mode), 0 if > (overwrite)
	int pipe_fd[2];
	struct s_command *next; // Next command in a pipeline (if `|` is used)
} t_command;

void ft_process(char *str, int *pipe_fd, int input_fd, t_env **head, t_dir *dir, t_stat *STATUS)
{
	pid_t id;
	char *str_cmd = strip_redirections(str);

	id = fork();
	if (id == 0)
	{
		// Child process
		if (input_fd != 0)
		{
			dup2(input_fd, STDIN_FILENO);
			close(input_fd);
		}

		// Only redirect output if this is part of a pipeline
		if (pipe_fd)
		{
			dup2(pipe_fd[1], STDOUT_FILENO);
			close(pipe_fd[1]);
			close(pipe_fd[0]);
		}

		check_redirections(str);
		execute_command(str_cmd, *head, dir, STATUS);
		exit(dir->exit_status_);
	}
	else if (id > 0)
	{
		// Parent process
		if (pipe_fd)
		{
			close(pipe_fd[1]); // Close write end
		}
		if (input_fd != 0)
		{
			close(input_fd);
		}

		// For single commands, wait immediately
		if (!pipe_fd)
		{
			int status;
			waitpid(id, &status, 0);
			if (WIFEXITED(status))
				dir->exit_status_ = WEXITSTATUS(status);
			update_directory(dir, *head);
		}
	}
	free(str_cmd);
}

void wait_for_children(int num_children)
{
	int status;
	int i;

	i = 0;
	while (i < num_children)
	{
		wait(&status); // must use waitpid
		i++;
	}
}

void execve_input(char *line, char **split, t_env **head, t_dir *dir, t_stat *STATUS)
{
	int num_commands = 0;
	int input_fd = 0;
	int pipe_fd[2];
	int *child_pids = NULL;

	split = ft_split(line, '|');
	if (!split)
		return;

	// Count commands
	for (num_commands = 0; split[num_commands]; num_commands++)
		;

	child_pids = malloc(num_commands * sizeof(int));
	if (!child_pids)
	{
		free_array(split);
		return;
	}

	// Execute each command in pipeline
	for (int i = 0; split[i]; i++)
	{
		if (i < num_commands - 1 && pipe(pipe_fd) < 0)
		{
			perror("pipe");
			dir->exit_status_ = 1;
			break;
		}

		child_pids[i] = fork();
		if (child_pids[i] == 0)
		{ // Child
			ft_process(split[i], (i < num_commands - 1) ? pipe_fd : NULL,
					   input_fd, head, dir, STATUS);
			exit(dir->exit_status_);
		}
		else if (child_pids[i] > 0)
		{ // Parent
			if (i < num_commands - 1)
			{
				close(pipe_fd[1]); // Close write end
				if (input_fd != 0)
					close(input_fd);
				input_fd = pipe_fd[0]; // Next command reads from here
			}
		}
		else
		{
			perror("fork");
			dir->exit_status_ = 1;
			break;
		}
	}

	// Wait for all children and get final status
	for (int i = 0; i < num_commands; i++)
	{
		if (child_pids[i] > 0)
		{
			int status;
			waitpid(child_pids[i], &status, 0);
			if (i == num_commands - 1)
			{ // Last command determines status
				if (WIFEXITED(status))
					dir->exit_status_ = WEXITSTATUS(status);
				else if (WIFSIGNALED(status))
					dir->exit_status_ = 128 + WTERMSIG(status);
			}
		}
	}

	// Clean up
	if (input_fd != 0)
		close(input_fd);
	free(child_pids);
	free_array(split);
	update_directory(dir, *head); // Update after all commands complete
}

int main(int ac, char **av, char **env)
{
	char *line;
	t_signal signal;
	t_env *my_env;
	t_dir directory;
	t_stat STATUS;
	t_cmd *cmd;
	char **split;
	int i;

	(void)ac;
	(void)av;
	line = NULL;
	split = NULL;
	directory.dir = NULL;
	directory.oldir = NULL;
	directory.home = NULL;
	directory.exit_status_ = 0;
	STATUS = 0;

	setup_signals(&signal);
	my_env = create_env(env);
	update_directory(&directory, my_env);
	while (!STATUS)
	{
		signal_global = 0;
		line = readline("✧ mi/sh ➤ ");
		if(!*line)
			continue;
		if (signal_global == SIGINT)
			directory.exit_status_ = 130;
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
			free(line);
			continue;
		}
		tilda_remod(&line, directory);
		char *expanded = special_check(line, my_env, directory.exit_status_);
		if (expanded)
		{
			free(line);
			line = expanded;
		}
		split = parse_prompt_to_argv(line);
		if (line[0] == '\0' || !split || !split[0])
			directory.exit_status_ = 0;

		// merging ...
		if (!ft_strcmp(split[0], "cd"))
			handle_cd(split, my_env, &directory);
		else
			execve_input(line, split, &my_env, &directory, &STATUS);

		// else if (is_dir(split[0], &directory))
		//     handle_builtins(line, split, &directory, &STATUS, my_env);

		if (line)
			free(line);
		if (split)
			free_array(split);
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
