# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: zcadinot <zcadinot@student.42lehavre.      +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/10/19 18:05:52 by zcadinot          #+#    #+#              #
#    Updated: 2026/10/19 18:09:11 by zcadinot         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME        := ft_shield.exe

CC          := x86_64-w64-mingw32-gcc
CPPFLAGS    := -Iinclude
CFLAGS      := -O3 -Wall -Wextra -Werror -MMD -MP
LDFLAGS     := -lws2_32

SRC_DIR     := src
OBJ_DIR     := obj

# Recursively discover sources using GNU Make itself.  This avoids calling
# Windows' find.exe when the project is built from PowerShell.
rwildcard   = $(foreach entry,$(wildcard $1*),$(call rwildcard,$(entry)/,$2)$(filter $(subst *,%,$2),$(entry)))
SRCS        := $(call rwildcard,$(SRC_DIR)/,*.c)
OBJS        := $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
DEPS        := $(OBJS:.o=.d)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

-include $(DEPS)

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re

