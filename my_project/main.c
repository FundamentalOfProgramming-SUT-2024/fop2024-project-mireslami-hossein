/*
To Run The GAME compile it by entering this command in terminal:

gcc main.c login.c pregame.c game.c -o game.out -lncursesw -lcjson -lm && ./game.out

*/ 

#include <stdio.h>
#include <stdlib.h>
#include <ncursesw/ncurses.h>

#include "basic_loads.h"


#include "login.h"
#include "pregame.h"
#include "game.h"



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
    
    GameState state = STATE_LOGIN;  // شروع از صفحه لاگین
    int nextState;

    while (state != STATE_EXIT) {
        switch (state) {
            case STATE_LOGIN:
                nextState = load_first_page(game.user);
                break;
            case STATE_PREGAME:
                nextState = load_pregame_page(&game);
                break;
            case STATE_GAME:
                nextState = load_main_game(&game);
                break;
        }

        // END_GAME should change
        if (nextState == STATE_EXIT || nextState == STATE_END_GAME) {
            break;
        } else {
            state = nextState; // تغییر به وضعیت جدید
        }
    }

    return 0;

    endwin();
}