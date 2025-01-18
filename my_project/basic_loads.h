#ifndef BASIC_LOADS_H
#define BASIC_LOADS_H

#include<stdio.h>
#include<stdlib.h>
#include<ncurses.h>

// MAX_SIZES
#define MAX_USERNAME 15
#define MAX_PASSWORD 20

// New COLORS
#define COLOR_PURPLE 10
#define CUSTOM_RED 11
#define CUSTOM_CYAN 12
#define CUSTOM_GREEN 13
#define CUSTOM_ORANGE 14

// New PAIRS
#define BG_LOGIN 0
#define MESSAGE_COLOR 1
#define BTN_DEFAULT 2
#define BTN_SELECTED 3
#define HEADER_COLOR 4
#define TEXT_COLOR 5
#define LABEL_COLOR 5

void add_color_rgb(int id, int r, int g, int b){
    start_color();
    init_color(id, r*1000/255, g*1000/255, b*1000/255);
}

void clear_part(WINDOW* w, int y1, int x1, int y2, int x2){
    for(int j = y1; j <= y2; j++){
        for(int i = x1; i <= x2; i++){
            mvwaddch(w, j, i, ' ');
        }
    }
    wrefresh(w);
}

// string functions
bool is_upper(char c){
    return (c >= 'A' && c <= 'Z');
} 
bool is_lower(char c){
    return (c >= 'a' && c <= 'z');
}
bool is_int(char c){
    return (c >= '0' && c <= '9');
}

void game_initalize(){
    start_color();
    add_color_rgb(COLOR_PURPLE, 71, 50, 122);
    add_color_rgb(CUSTOM_RED, 203, 35, 44);
    add_color_rgb(CUSTOM_GREEN, 93, 255, 101);
    add_color_rgb(CUSTOM_CYAN, 151, 176, 243);
    add_color_rgb(CUSTOM_ORANGE, 255, 178, 131);

    init_pair(MESSAGE_COLOR, COLOR_RED, COLOR_BLACK);
    init_pair(BTN_DEFAULT, COLOR_CYAN, COLOR_BLACK);
    init_pair(BTN_SELECTED,COLOR_BLACK , COLOR_CYAN);
    init_pair(BG_LOGIN, COLOR_PURPLE, COLOR_PURPLE);
    init_pair(HEADER_COLOR, CUSTOM_CYAN, COLOR_BLACK);
    init_pair(TEXT_COLOR, COLOR_WHITE, COLOR_BLACK);
    init_pair(LABEL_COLOR, CUSTOM_ORANGE, COLOR_BLACK);
}

#endif