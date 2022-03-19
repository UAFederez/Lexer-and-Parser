SRC = src/main.c            \
	  src/Lexer.c           \
	  src/Parser.c          \
	  src/AST_Expression.c  \
	  src/AST_Declaration.c \
	  src/AST_Statement.c   \
	  src/GraphvizOutput.c

CC  = clang
CF  = -Wall -Wextra -Wpedantic -D_CRT_SECURE_NO_WARNINGS -g

all: $(SRC)
	$(CC) $(SRC) $(CF)

clean:
ifeq ($(OS), Windows_NT)
	del /S *o a.exe a.out
else
	$(DEL) $(DELF) $(BDIR)/*.o a.exe a.out
endif

