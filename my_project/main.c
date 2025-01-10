#include<stdio.h>
#include<stdlib.h>
#include<ncurses.h>

#include "basic_loads.c"
#include "login.c"

typedef struct
{
    int x,y;
} Loc;


int main(){
    initscr();
    cbreak();
    keypad(stdscr, TRUE);


    print_borders();

    printw("Hello!\n");
    if(!has_colors()){
        endwin();
        printf("Sorry! Your Terminal Doesn't support colors!\n");
        return 0;
    }
    refresh();
    int count = 0;
    while(1){
        mvprintw(1,1,"hey! count is %d\n", count);
        char c = getch();
        count++;
        if(c == 'q') break;
        refresh();
    }
    endwin();
}