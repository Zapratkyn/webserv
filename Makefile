NAME				=	webserv

CC					=	c++
CPPFLAGS			=	-Wall -Wextra -Werror -std=c++98
RM					=	rm -rf

OBJDIR				=	.obj

SRC					=	srcs/main.cpp \
						srcs/Server.cpp \
						srcs/Webserv.cpp

OBJ					=	$(addprefix $(OBJDIR)/, $(SRC:.cpp=.o))

all					=	$(NAME)

$(NAME):			$(OBJ)
		$(CC) $(CPPFLAGS) $(OBJ) -o $(NAME)

$(OBJDIR)/%.o: 		%.cpp
		@mkdir -p $(dir $@)
		@$(CC) $(CPPFLAGS) -c $< -o $@

clean:
		$(RM) $(OBJDIR) $(OBJ)

fclean:				clean
		$(RM) $(NAME)

re:		fclean $(NAME)

.PHONY : all clean fclean re