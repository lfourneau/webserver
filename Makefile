# -*- Makefile -*-

CC=c++
FLAGS=-Wall -Wextra -Werror -std=c++98 -g
SRC_DIR=src
OBJ_DIR=obj
CPP=$(wildcard $(SRC_DIR)/*.cpp)
CO=$(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(CPP))
DEPS=$(CO:.o=.d)
PROG=webserv

all: $(PROG)

$(PROG): $(CO)
	$(CC) $(FLAGS) $(CO) -o $@

$(OBJ_DIR)/%.o:$(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CC) $(FLAGS) -MMD -MP $< -c -o $@

$(OBJ_DIR):
	mkdir -p $@

clean:
	rm -rf $(CO) $(DEPS)

fclean: clean
	rm -rf $(PROG)

re: fclean all

.PHONY: all clean fclean re

-include $(DEPS)
