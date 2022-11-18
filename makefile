test.out:
	gcc -Wall -Wextra -o test minishell.c libparser.a -static
	