# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: iromero- <iromero-@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2021/02/09 12:32:45 by mlaplana          #+#    #+#              #
#    Updated: 2021/04/20 16:30:10 by iromero-         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = webserv

CC = clang++

CC_FLAGS = -Wall -Wextra -Werror

RM = rm -rf

DIR_SRCS = ./srcs/

DIR_HEADERS = ./includes/

DIR_OBJS = ./compiled_srcs/

SRC =	client.cpp	conf.cpp	main.cpp \
		methods.cpp	request.cpp	server.cpp \
		utils.cpp get_next_line.cpp get_next_line_utils.cpp

OBJS = $(SRC:%.cpp=$(DIR_OBJS)%.o)

all: $(NAME)

$(NAME): $(OBJS)
		@echo Compiling $(NAME)
		@$(CC) $(CC_FLAGS) -I $(DIR_HEADERS) $(OBJS) ${OBJGNL} -o $(NAME)
		@echo ======[Done]======

$(OBJS):		| $(DIR_OBJS)

$(DIR_OBJS)%.o: $(DIR_SRCS)%.cpp
				@$(CC) $(CC_FLAGS) -I $(DIR_HEADERS) -c $< -o $@

$(DIR_OBJS):
				@mkdir $(DIR_OBJS)

clean:
				@$(RM) $(DIR_OBJS)

fclean:			clean
				@$(RM) $(NAME)
				
re:				fclean all
		
.PHONY: clean fclean all re