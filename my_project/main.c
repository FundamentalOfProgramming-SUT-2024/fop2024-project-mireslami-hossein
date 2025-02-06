/*
To Run The GAME compile it by entering this command in terminal:

gcc main.c -o game.out -lncursesw -lcjson -lm && ./game.out

*/ 

#include <stdio.h>
#include <stdlib.h>
#include <ncursesw/ncurses.h>

#include "basic_loads.h"
#include "login.c"
#include "pregame.c"



int main(){
    setlocale(LC_ALL, "");
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
    
    Game game;

    User user;
    reset_user_data(&user);

    game.user = &user;
    
    game_initalize();
    
    load_first_page(&user);
    load_pregame_page(&game);

    endwin();
}