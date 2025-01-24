/*
To Run The GAME compile it by entering this command in terminal:

gcc main.c -o game.out -lncurses -lcjson && ./game.out

*/ 

#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>

#include "basic_loads.h"
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
    curs_set(FALSE);

    if(!has_colors()){
        endwin();
        printf("Sorry! Your Terminal Doesn't support colors!\n");
        return 0;
    }

    game_initalize();
    User user;
    reset_user_data(&user);

    load_first_page(&user);
    
    refresh();
    
    endwin();
}