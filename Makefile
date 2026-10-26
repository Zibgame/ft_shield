# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: zcadinot <zcadinot@student.42lehavre.      +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/10/19 18:05:52 by zcadinot          #+#    #+#              #
#    Updated: 2026/10/22 16:16:36 by zcadinot         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME        := ft_shield.exe

JOBS        := $(shell nproc 2>/dev/null || echo 4)
ifeq ($(MAKELEVEL),0)
ifeq ($(filter -j% --jobs%,$(MAKEFLAGS)),)
MAKEFLAGS   += -j$(JOBS)
endif
endif

CC          := x86_64-w64-mingw32-gcc
WINDRES     := $(shell command -v x86_64-w64-mingw32-windres 2>/dev/null || echo /sgoinfre/goinfre/Perso/zcadinot/.home/.Apps/llvm-mingw/bin/x86_64-w64-mingw32-windres)
CPPFLAGS    := -Iinclude -D_WIN32_WINNT=0x0A00
CFLAGS      := -O3 -pipe -Wall -Wextra -Werror -MMD -MP
LDFLAGS     := -lws2_32 -lbcrypt -mwindows

SRC_DIR     := src
OBJ_DIR     := obj
RC          := $(SRC_DIR)/version.rc
RC_OBJ      := $(OBJ_DIR)/version.o

rwildcard   = $(foreach entry,$(wildcard $1*),$(call rwildcard,$(entry)/,$2)$(filter $(subst *,%,$2),$(entry)))
SRCS        := $(call rwildcard,$(SRC_DIR)/,*.c)
OBJS        := $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
DEPS        := $(OBJS:.o=.d)

all: $(NAME)
	@echo "==> Build OK: $(NAME)"

$(NAME): $(OBJS) $(RC_OBJ)
	$(CC) $(OBJS) $(RC_OBJ) $(LDFLAGS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(RC_OBJ): $(RC)
	@mkdir -p $(OBJ_DIR)
	$(WINDRES) $< $@

-include $(DEPS)

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME) $(NAME).obsidian.tmp

TOOLS_DIR   := tools
OBSIDIAN_REPO := https://github.com/Zibgame/Obsidian-Packer.git
OBSIDIAN_DIR := $(TOOLS_DIR)/Obsidian-Packer
OBSIDIAN    := $(OBSIDIAN_DIR)/obsidian
OBSIDIAN_STUB := $(OBSIDIAN_DIR)/build/obsidian-stub.exe
OBSIDIAN_STAMP := $(OBSIDIAN_DIR)/.ft-shield-installed
MINGW_BIN   ?= /sgoinfre/goinfre/Perso/$(USER)/.home/.Apps/llvm-mingw/bin
PACKED_TMP  := $(NAME).obsidian.tmp

re: fclean
	$(MAKE) all

install-packer: $(OBSIDIAN_STAMP)

$(OBSIDIAN_STAMP):
	@mkdir -p "$(TOOLS_DIR)"
	@test -e "$(OBSIDIAN_DIR)" || git clone --depth 1 "$(OBSIDIAN_REPO)" "$(OBSIDIAN_DIR)"
	$(MAKE) -C "$(OBSIDIAN_DIR)" parallel CFLAGS="-O2 -Wall -D_7ZIP_ST -DZ7_ST -fPIE"
	$(MAKE) -C "$(OBSIDIAN_DIR)" stub-mingw \
		MINGW_CXX="$(MINGW_BIN)/x86_64-w64-mingw32-g++" \
		MINGW_CC="$(MINGW_BIN)/x86_64-w64-mingw32-gcc"
	objcopy --subsystem windows "$(OBSIDIAN_STUB)"
	@touch "$@"

pack: all install-packer
	$(OBSIDIAN) -o $(PACKED_TMP) $(OBSIDIAN_STUB) $(NAME)
	mv $(PACKED_TMP) $(NAME)
	@echo "==> Packed with Obsidian: $(NAME)"

opack: pack

.PHONY: all clean fclean re install-packer pack opack

