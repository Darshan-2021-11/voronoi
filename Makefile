FLAGS =-Wall -Werror -Wextra -Wpedantic 

voronoi: main.c
	cc $(FLAGS) -o $@ $<
