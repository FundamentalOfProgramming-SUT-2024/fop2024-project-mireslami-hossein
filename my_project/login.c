#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include "basic_loads.h"


char login_messages[2][50] = {
    "Welcome!", "Choose a way to continue: (press Q to exit)"
};

char login_items[2][50] = {
    "Sign Up", "Login"
};

void print_messages(WINDOW* w, char ms[][50], int size, int y, int x){
    attron(COLOR_PAIR(MESSAGE_COLOR));
    for(int i = 0; i < size; i ++){
        int ms_len = strlen(ms[i]);
        int x_m = x - ms_len/2;
        int y_m = y + i*2 + 1;
        mvwprintw(w,y_m, x_m, "%s", ms[i]);
    }
    wrefresh(w);
    attroff(COLOR_PAIR(MESSAGE_COLOR));
}

void print_buttons(WINDOW* w, char btns[][50], int size,int selected, int y, int x){
    attron(A_BOLD);
    for(int i = 0; i < size; i ++){
        int btn_len = strlen(btns[i]);
        int x_m = x - btn_len/2;
        int y_m = y + i*2 + 1;
        if(i == selected){
            attron(A_REVERSE);
            mvwprintw(w,y_m, x_m, "%s", btns[i]);
            attroff(A_REVERSE);
        } else{
            attron(COLOR_PAIR(BUTTONS_DEFAULT));
            mvwprintw(w,y_m, x_m, "%s", btns[i]);
            attroff(COLOR_PAIR(BUTTONS_DEFAULT));
        }
    }
    wrefresh(w);
    attroff(COLOR_PAIR(BUTTONS_DEFAULT));
    attroff(A_BOLD);
}


void handle_selected_btn(int* selected, int size, int* flag){
    int ch = getch();
    switch (ch)
    {
    case KEY_UP:
        (*selected)--;
        if(*selected < 0) *selected = size - 1;
        break;
    case KEY_DOWN:
        (*selected)++;
        if(*selected >= size) *selected = 0;
        break;
    
    case KEY_ENTER:
        *flag = 1;
        break;
    
    case 'q':
        *flag = -1;
    }
}

void open_form(int selected){
    mvprintw(LINES/2, COLS/2, "WELCOME TO SECOND PAGE!!!");
}

void load_login_page(){
    clear();

    start_color();
    game_initalize();

    int selected_btn = 0;
    int exit_flag = 0;
    int btn_nums = 2;

    int height = 18, width = 25;
    int y = LINES/2 - height/2;
    int x = COLS/2 - width/2;
    WINDOW* menu = newwin(height , width,y,x);
    box(menu, 0, 0);
    refresh();
    
    while(!exit_flag){
        print_messages(menu, login_messages, 2, 1, width/2);
        print_buttons(menu, login_items, btn_nums, selected_btn, 7, width/2);
        
        handle_selected_btn(&selected_btn, btn_nums, &exit_flag);
    }

    if(exit_flag == -1){
        endwin();
    }

    delwin(menu);
    open_form(selected_btn);
}