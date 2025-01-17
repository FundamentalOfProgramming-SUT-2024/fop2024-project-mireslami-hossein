#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include "cjson/cJSON.h"
#include "basic_loads.h"


char login_messages[3][50] = {
    "Welcome!", "Choose a way to continue:", "(Q to exit)"
};

char login_items[2][50] = {
    "Sign Up", "Login"
};

char signup_form_labels[4][50] = {
    "Username: ", "Password: ", "Email: ", "Checker Word (recommended): "
};

char login_form_labels[4][50] = {
    "Username: ", "Password: "
};

void print_messages(WINDOW* w, char ms[][50], int size, int y, int x){
    wattron(w, COLOR_PAIR(HEADER_COLOR));
    for(int i = 0; i < size; i ++){
        int ms_len = strlen(ms[i]);
        int x_m = x - ms_len/2;
        int y_m = y + i*2 + 1;
        mvwprintw(w,y_m, x_m, "%s", ms[i]);
    }
    wrefresh(w);
    wattroff(w, COLOR_PAIR(HEADER_COLOR));
}

void print_buttons(WINDOW* w, char btns[][50], int size,int selected, int y, int x){
    wattron(w, A_BOLD);
    
    for(int i = 0; i < size; i ++){
        int btn_len = strlen(btns[i]);
        int x_m = x - btn_len/2;
        int y_m = y + i*2 + 1;
        if(i == selected){
            wattron(w, COLOR_PAIR(BTN_SELECTED));
            mvwprintw(w,y_m, x_m, "%s", btns[i]);
            wattroff(w, COLOR_PAIR(BTN_SELECTED));
        } else{
            wattron(w, COLOR_PAIR(BTN_DEFAULT));
            mvwprintw(w,y_m, x_m, "%s", btns[i]);
            wattroff(w, COLOR_PAIR(BTN_DEFAULT));
        }
    }
    wrefresh(w);
    wattroff(w, A_BOLD);
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
    
    case 10: case 13:
        *flag = 1;
        break;
    
    case 'q':
        *flag = -1;
        break;
    }
}


// Validation
void print_error_message(WINDOW* w, int height, int width, char* ms){
    wattron(w, COLOR_PAIR(MESSAGE_COLOR) | A_BOLD);
    mvwprintw(w, height - 2 ,width/2 - strlen(ms), "%s", ms);
    wattroff(w, COLOR_PAIR(MESSAGE_COLOR)| A_BOLD);
}

bool does_user_exist(char name[]){
    // Most be completed
    char usernames[50][50] = {"ali", "hossein"};
    int number_of_users = 2;
    for(int i = 0; i < number_of_users; i++){
        if(strcmp(name, usernames[i])){
            return TRUE;
        }
    }
    return FALSE;
}

void get_username(WINDOW* sign_form, int height, int width, char* username){
    int x_start_label = 6;
    int y = 7;

    while(1){
        print_error_message(sign_form, height, width, "Enter your username");
        
        wattron(sign_form, COLOR_PAIR(LABEL_COLOR) | A_BOLD);
        mvwprintw(sign_form, y, x_start_label, "%s", login_form_labels[0]);
        wattroff(sign_form, COLOR_PAIR(LABEL_COLOR)| A_BOLD);
        curs_set(TRUE);
        echo();
        wrefresh(sign_form);

        wgetnstr(sign_form , username, MAX_USERNAME);

        if(does_user_exist(username)){
            print_error_message(sign_form, height, width, "This User name already taken!");
            int x_end_of_username = x_start_label + strlen(login_form_labels[0]) + strlen(username);
            wmove(sign_form, y, x_end_of_username);
        } else{
            char text[50] = "username %s added successfully!";
            print_error_message(sign_form, height, width, text);
            break;
        }
    }
}

void signup_user(){
    int width = 46, height = 26;
    int y = LINES/2 - height/2;
    int x = COLS/2 - width/2;
    WINDOW* sign_form = newwin(height, width, y, x);
    box(sign_form, 0, 0);
    refresh();
    
    char title[20] = "Sign Up";
    start_color();
    wattron(sign_form, COLOR_PAIR(HEADER_COLOR) | A_BOLD);
    mvwprintw(sign_form,3,width/2 - strlen(title)/2, "%s",title);
    wattroff(sign_form, COLOR_PAIR(HEADER_COLOR)| A_BOLD);
    
    char *username = (char *)malloc(MAX_USERNAME * sizeof(char));
    get_username(sign_form, height, width, username);  
    

    
    refresh();
    wrefresh(sign_form);


    int c = getch();
}

void open_form(int selected){
    clear();
    // Sign Up
    if(selected == 0){
        signup_user();
    } else if(selected == 1){
        // login_user();
    }
    refresh();
}

void load_first_page(){
    clear();

    start_color();
    game_initalize();

    int selected_btn = 0;
    int exit_flag = 0;
    int btn_nums = 2;
    int message_num = 3;

    int height = 18, width = 35;
    int y = LINES/2 - height/2;
    int x = COLS/2 - width/2;
    WINDOW* menu = newwin(height , width,y,x);
    wrefresh(menu);
    box(menu, 0, 0);
    refresh();

    
    while(!exit_flag){
        print_messages(menu, login_messages, message_num, 1, width/2);
        print_buttons(menu, login_items, btn_nums, selected_btn, 9, width/2);
        
        handle_selected_btn(&selected_btn, btn_nums, &exit_flag);
    }

    if(exit_flag == -1){
        endwin();
        exit(0);
    }

    delwin(menu);
    open_form(selected_btn);
}