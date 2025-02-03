#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ncurses.h>
#include "cjson/cJSON.h"
#include <regex.h>
#include <time.h>

#include "basic_loads.h"

void draw_map(WINDOW* win, int h, int w, Game* g){
    for(int i = 0; i < g->map->rooms_num; i++){
        Room r = g->map->rooms[i];
        mvwaddch(win, r.e1.y, r.e1.x, '.');
        mvwaddch(win, r.e2.y, r.e2.x, '.');
        mvwaddch(win, r.e3.y, r.e3.x, '.');
        mvwaddch(win, r.e4.y, r.e4.x, '.');
        for(int j = r.e1.x + 1; j < r.e2.x; j++){
            mvwaddch(win, r.e1.y, j, '_');
            mvwaddch(win, r.e4.y, j, '_');
        }
        for(int j = r.e1.y + 1; j < r.e3.y; j++){
            mvwaddch(win, j, r.e1.x, '|');
            mvwaddch(win, j, r.e4.x, '|');
        }
        for(int j = r.e1.y + 1; j < r.e3.y; j++){
            for(int z = r.e1.x + 1; z < r.e2.x; z++){
                mvwaddch(win, j, z, '.');
            }
        }
    }
    wrefresh(win);
}

void make_random_map(Game* g){
    int width = COLS - 22, height = 30;
    int x_w = 1;
    int y_w = LINES/2- height/2;

    WINDOW* main_game = newwin(height , width, y_w,x_w);
    
    refresh();
    box(main_game, 0,0);
    wrefresh(main_game);
    Map* map = malloc(sizeof(Map));
    g->map = map;

    int rooms_num = rand_in(6,9);
    map->rooms_num = rooms_num;

    Room* map_rooms = malloc(rooms_num * sizeof(Room));
    for(int i = 0; i < rooms_num; i++){
        int x_s = (width/4) * (i%4) + 1;
        int x_f = (width/4) * (i%4 + 1) - 1;
        int y_s = (height/2) * (i/4) + 1;
        int y_f = (height/2) * (i/4 + 1) - 1;
        
        int w = rand_in(5,10);
        int h = rand_in(5,10);

        map_rooms[i].e1.x = rand_in(x_s , x_f - w);
        map_rooms[i].e1.y = rand_in(y_s , y_f - h);
        map_rooms[i].w = w;
        map_rooms[i].h = h;
        
        map_rooms[i].e2.x = map_rooms[i].e1.x + w;
        map_rooms[i].e2.y = map_rooms[i].e1.y;
        
        map_rooms[i].e3.x = map_rooms[i].e1.x;
        map_rooms[i].e3.y = map_rooms[i].e1.y + h;
        
        map_rooms[i].e4.x = map_rooms[i].e1.x + w;
        map_rooms[i].e4.y = map_rooms[i].e1.y + h;
    }
    g->map->rooms = map_rooms;
    // for(int i = 0; i < rooms_num; i++){
    //     printw("x%d: %d, y%d: %d, w: %d, h: %d\n", i, map_rooms[i].e1.x, i, map_rooms[i].e1.y, map_rooms[i].w, map_rooms[i].h);
    // }
    
    draw_map(main_game, height, width, g);
    getch();
    delwin(main_game);
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

    make_random_map(g);
    
}