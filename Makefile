GREEN = \033[32m
BLUE = \033[34m
RESET = \033[0m

NAME = webserv
CC = c++
CFLAGS = -Wall -Wextra -Werror -std=c++98
CFILES = main.cpp Core.cpp HttpRequest.cpp HttpResponse.cpp \
		 Server.cpp Client.cpp File.cpp
OBJS_DIR = ./objs/
SRC_DIR = ./src/
TEST_DIR = ./test/
OBJS = ${addprefix ${OBJS_DIR}, ${CFILES:.cpp=.o}}
VPATH = ${SRC_DIR}:${TEST_DIR}

all: ${NAME}

${NAME}: ${OBJS}
	@${CC} ${CFLAGS} ${OBJS} -o $@
	@echo "${GREEN}${NAME} compiled ${RESET}"

${OBJS_DIR}:
	@mkdir objs

${OBJS}: | ${OBJS_DIR}

${OBJS_DIR}%.o: %.cpp
	@${CC} ${CFLAGS} -c $^ -o $@

clean:
	@rm -rf ${OBJS_DIR}
	@echo "${BLUE}objects deleted ${RESET}"

fclean: clean
	@rm -rf ${NAME}
	@echo "${BLUE}${NAME} deleted ${RESET}"

re: fclean all

.PHONY: re clean fclean all
