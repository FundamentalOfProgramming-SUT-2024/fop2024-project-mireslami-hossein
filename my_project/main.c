/*
To Run The GAME compile it by entering this command in terminal:

gcc main.c -o game.out -lncurses -lcjson -lm && ./game.out

*/ 

#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>

#include "basic_loads.h"
#include "login.c"
#include "pregame.c"
#include "game.c"



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
    
    Game game;

    User user;
    reset_user_data(&user);

    game.user = &user;
    Player player;
    game.player = &player;
    
    game_initalize();
    
    load_first_page(&user);
    load_pregame_page(&user, &player, &game);

    // load_main_game();
    endwin();
}