#include<stdio.h>
#include<stdlib.h>
#include<ncurses.h>

void print_borders(){
    for(int i = 0; i < LINES; i++){
        mvprintw(i,0, "|");
        mvprintw(i,COLS - 1, "|");
    }
    for(int i = 0; i < COLS; i++){
        mvprintw(0, i, "_");
        mvprintw(LINES - 1 ,i, "_");
    }
    refresh();
    move(2, 2);
}