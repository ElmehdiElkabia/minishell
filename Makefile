NAME = minishell
CC = cc
CFLAGS = -Wall -Wextra -Werror -g
LDFLAGS = -lreadline
SRCS = src/builtins.c  src/env.c  src/executor.c  \
		src/init.c  src/main.c  src/parser.c  src/pipes.c \
		  src/redirections.c  src/signals.c  src/utils.c
LIBFT_DIR = ./libft
LIB = $(LIBFT_DIR)/libft.a
OBJS = $(SRCS:.c=.o)

all: $(NAME)

$(LIB):
	make -C $(LIBFT_DIR)

$(NAME): $(OBJS) $(LIB)
	$(CC) $(OBJS) $(LIB) -o $(NAME) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all
