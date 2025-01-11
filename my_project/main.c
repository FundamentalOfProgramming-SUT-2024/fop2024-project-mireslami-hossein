/*
To Run The GAME compile it by entering this command in terminal:

gcc main.c -o game.out -lncurses && ./game.out

*/ 

#include<stdio.h>
#include<stdlib.h>
#include<ncurses.h>

// #include "basic_loads.h"
#include "login.c"

typedef struct
{
    int x,y;
} Loc;


int main(){
    initscr();
    cbreak();
    keypad(stdscr, TRUE);
    noecho();

    // game_initalize();
    load_login_page();

    if(!has_colors()){
        endwin();
        printf("Sorry! Your Terminal Doesn't support colors!\n");
        return 0;
    }
    refresh();
    
    endwin();
}