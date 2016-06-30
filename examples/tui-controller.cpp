#include <ncurses.h>

int main(int argc, char *args[]) {

	initscr();

	printw("hello");

	getch();

	endwin();
    
    return 0;
}
