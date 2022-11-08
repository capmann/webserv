SRCS	=	srcs/main.cpp\
			srcs/Sockets/SimpleSocket.cpp\
			srcs/Sockets/ConnectingSocket.cpp\
			srcs/Sockets/BindingSocket.cpp\
			srcs/Sockets/ListeningSocket.cpp\
			srcs/Server/SimpleServer.cpp\
			srcs/Server/httpServer.cpp\
			srcs/Config/parseConfig.cpp\
			srcs/Request/httpRequest.cpp\
			srcs/Response/httpResponse.cpp

OBJS	=	${SRCS:.c=.o}

CXX		=	g++

CFLAGS	=	-Wall -Werror -Wextra -std=c++98 

NAME	=	webserv

${NAME}	:	${OBJS}
		${CXX} ${CFLAGS} ${OBJS} -o ${NAME} 

all		:	${NAME}

clean	:

fclean	:	clean
			rm -rf ${NAME}
			$(MAKE) -C . clean

re	:	fclean all

.PHONY	:	all clean fclean re
