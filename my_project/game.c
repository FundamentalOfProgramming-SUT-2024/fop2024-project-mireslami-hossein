#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ncurses.h>
#include "cjson/cJSON.h"
#include <regex.h>
#include <time.h>

#include "basic_loads.h"

typedef struct 
{
    Loc e1;
    Loc e4;
    Loc door1;
    Loc door2;
} room;



void make_random_map(){
    int width = 100, height = 30;
    int x_w = COLS/2- width/2;
    int y_w = LINES/2- height/2;

    WINDOW* main_game = newwin(height , width,y_w,x_w);
    
    refresh();
    box(main_game, 0,0);
    wrefresh(main_game);

    int rooms_num = rand_in(4, 6);
    
    delwin(main_game);
    getch();
}

void load_main_game(Game* g){
    clear();

    start_color();
    game_initalize();
    // int height = 18, width = 40;
    // int y_w = LINES/2 - height/2;
    // int x_w = COLS/2 - width/2;
    // WINDOW* menu = newwin(height , width,y_w,x_w);
    // wrefresh(menu);
    // box(menu, 0, 0);
    // refresh();

    // int selected = 0;
    // int pressed = 0;

    
    // delwin(menu);

    make_random_map();
}