#include<stdio.h>
#include<stdlib.h>
#include<ncurses.h>

// New COLORS
#define COLOR_PURPLE 10
#define COLOR_CUSTOM_RED 11
#define COLOR_CUSTOM_CYAN 12

// New PAIRS
#define BG_LOGIN 0
#define MESSAGE_COLOR 1
#define BUTTONS_DEFAULT 2

void game_initalize(){
    init_color(COLOR_PURPLE, 71, 50, 122);
    init_color(COLOR_CUSTOM_RED, 203, 35, 44);
    init_color(COLOR_CUSTOM_CYAN, 151, 176, 243);

    init_pair(MESSAGE_COLOR, COLOR_RED, COLOR_BLACK);
    init_pair(BUTTONS_DEFAULT, COLOR_CYAN, COLOR_BLACK);
    init_pair(BG_LOGIN, COLOR_WHITE, COLOR_PURPLE);
}