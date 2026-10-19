NAME = ft_shield

CC = gcc
CFLAGS = -Wall -Wextra -Werror -O2
INCLUDES = -I include
SRCDIR = src
OBJDIR = obj

SRCS = $(SRCDIR)/main.c \
       $(SRCDIR)/daemon.c \
       $(SRCDIR)/network.c \
       $(SRCDIR)/auth.c \
       $(SRCDIR)/shell.c \
       $(SRCDIR)/persistence.c \
       $(SRCDIR)/stealth.c

OBJS = $(SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJDIR):
	mkdir -p $@

clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
