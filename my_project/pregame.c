#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include "cjson/cJSON.h"
#include <regex.h>
#include <time.h>

#include "basic_loads.h"

char pregame_user_options[5][50] = {
    "New Game", "Load Game", "Leaderboard", "Settings" , "Profile"
};
char pregame_guest_options[4][50] = {
    "New Game", "Leaderboard", "Settings" , "Profile"
};

char lb_titles[5][20] = {
    "Rank", "Username", "Total Score", "Total Games", "Experience"
};

// Leaderboard
void print_headers(WINDOW* w, char lb_titles[5][20], int size, int y, int x){
    wattron(w, A_BOLD | COLOR_PAIR(LABEL_COLOR));
    mvwprintw(w, y, x, "%s", lb_titles[0]);
    mvwprintw(w, y, x + 12, "%s", lb_titles[1]);
    mvwprintw(w, y, x + 28, "%s", lb_titles[2]);
    mvwprintw(w, y, x + 42, "%s", lb_titles[3]);
    mvwprintw(w, y, x + 56, "%s", lb_titles[4]);
    wattroff(w, A_BOLD | COLOR_PAIR(LABEL_COLOR));
    wrefresh(w);
    refresh();
}

void show_leaderboard(User user){
    clear();
    int height = 22, width = 75;
    int y_w = LINES/2 - height/2;
    int x_w = COLS/2 - width/2;
    WINDOW* lb_table = newwin(height , width,y_w,x_w);
    wrefresh(lb_table);
    box(lb_table, 0, 0);
    refresh();

    int x = width/2;
    int y_start = 6;
    int x_start = 4;
    print_title(lb_table, "LeaderBoard", 2, x);
    print_headers(lb_table, lb_titles, 5, y_start, x_start);

    // print_users();
    getch();
}


void load_pregame_page(User* user){
    clear();

    start_color();
    game_initalize();
    int height = 18, width = 40;
    int y_w = LINES/2 - height/2;
    int x_w = COLS/2 - width/2;
    WINDOW* menu = newwin(height , width,y_w,x_w);
    wrefresh(menu);
    box(menu, 0, 0);
    refresh();

    int selected = 0;
    int pressed = 0;

    print_title(menu, "Game Menu", 2, width/2);
    // BACK hint
    char ms[2][50] = {"Hint:", "Press ESC anywhere to go back!"};
    int x = width/2;
    int y = 6;
    
    wattron(menu, COLOR_PAIR(TEXT_COLOR));
    wattron(menu, A_BOLD | A_ITALIC);
    mvwprintw(menu, y-2, 3,"%s", ms[0]);
    wattroff(menu, A_BOLD | A_ITALIC);
    mvwprintw(menu, y-2, 9,"%s", ms[1]);
    wattroff(menu, COLOR_PAIR(TEXT_COLOR));
    wrefresh(menu);
 
    if(user->is_guest){
        while(!pressed){
            print_buttons(menu, pregame_guest_options, 4, selected, y, x, 2);
            handle_selected_btn(&selected, 4, &pressed);
        }
        if(selected == 1){
            show_leaderboard(*user);
        }
    } else{
        while(!pressed){
            print_buttons(menu, pregame_user_options, 5, selected, y, x, 2);
            handle_selected_btn(&selected, 5, &pressed);
        }
        if(selected == 2){
            show_leaderboard(*user);
        }
    }
    
    delwin(menu);
}