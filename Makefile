# -*- Makefile -*-

CC=c++
FLAGS=-Wall -Wextra -Werror -std=c++98 -g
SRC_DIR=src
OBJ_DIR=obj
CPP=$(wildcard $(SRC_DIR)/*.cpp)
CO=$(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(CPP))
# -MMD -MP : le compilateur ecrit un .d par fichier avec les headers inclus,
# un .o est donc recompile quand un header qu'il utilise change
DEPS=$(CO:.o=.d)
PROG=webserv

all: $(PROG)

$(PROG): $(CO)
	$(CC) $(FLAGS) $(CO) -o $@

$(OBJ_DIR)/%.o:$(SRC_DIR)/%.cpp
	$(CC) $(FLAGS) -MMD -MP $< -c -o $@

clean:
	rm -rf $(CO) $(DEPS)

fclean: clean
	rm -rf $(PROG)

re: fclean all

.PHONY: all clean fclean re

-include $(DEPS)
