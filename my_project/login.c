#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include "cjson/cJSON.h"
#include <regex.h>
#include <time.h>

#include "basic_loads.h"

typedef struct {
    char* username;
    char* password;
    char* email;
    char* checker_w;
} User;

char login_messages[3][50] = {
    "Welcome!", "Choose a way to continue:", "(Q to exit)"
};

char login_items[2][50] = {
    "Sign Up", "Login"
};

char signup_form_labels[4][50] = {
    "Username*: ", "Password*: ", "Email*: ", "Checker Word: "
};

char signup_form_buttons[2][50] = {
    "Generate Random Password", "submit"
};

char login_form_labels[4][50] = {
    "Username: ", "Password: "
};

void print_messages(WINDOW* w, char ms[][50], int size, int y, int x, char type, int color_id){
    wattron(w, COLOR_PAIR(color_id));
    if(type == 'c'){
        for(int i = 0; i < size; i++){
            int ms_len = strlen(ms[i]);
            int x_m = x - ms_len/2;
            int y_m = y + i*2;
            mvwprintw(w,y_m, x_m, "%s", ms[i]);
        }
    } else if(type == 'r'){
        for(int i = 0; i < size; i++){
            int ms_len = strlen(ms[i]);
            int x_m = x - ms_len;
            int y_m = y + i*2;
            mvwprintw(w,y_m, x_m, "%s", ms[i]);
        }
    } else if(type == 'l'){
        for(int i = 0; i < size; i++){
            int x_m = x;
            int y_m = y + i*2;
            mvwprintw(w,y_m, x_m, "%s", ms[i]);
        }
    }
    wrefresh(w);
    wattroff(w, COLOR_PAIR(color_id));
}

void print_buttons(WINDOW* w, char btns[][50], int size, int selected, int y, int x){
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


int handle_selected_btn(int* selected, int size, int* flag){
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
    
    // on press Enter
    case 10: case 13:
        *flag = 1;
        break;
    
    case 'q':case 'Q':
        *flag = -1;
        break;
    }
}


// Validation
void print_error_message(WINDOW* w, int height, int width,  int reset_y, int reset_x, char* ms){
    clear_part(w, height-2, 1, height-2, width - 2);
    wattron(w, COLOR_PAIR(MESSAGE_COLOR) | A_BOLD);
    mvwprintw(w, height - 2 , width/2 - strlen(ms)/2, "%s", ms);
    wattroff(w, COLOR_PAIR(MESSAGE_COLOR)| A_BOLD);
    wrefresh(w);
    wmove(w,reset_y, reset_x);
}

bool does_user_exist(char* name){
    // Most be completed
    char usernames[MAX_USERNAME][50] = {"ali", "hossein"};
    int number_of_users = 2;
    for(int i = 0; i < number_of_users; i++){
        if(strcmp(name, usernames[i]) == 0){
            return TRUE;
        }
    }
    return FALSE;
}

bool password_validated(WINDOW* w, int height, int width, int y, int x, char* password){
    bool min_len = FALSE, int_included = FALSE,
         lower_included = FALSE, upper_included = FALSE;
    if(strlen(password) < 7){
        print_error_message(w, height, width, y, x, "Password must be at least 7 characters!");
        return FALSE;
    } else{
        min_len = TRUE;
    }
    for(int i = 0; i < strlen(password); i++){
        if(is_upper(password[i])) upper_included = TRUE;
        if(is_lower(password[i])) lower_included = TRUE;
        if(is_digit(password[i])) int_included = TRUE;
    }

    if(!upper_included){
        print_error_message(w, height, width, y, x, "Password must have at least 1 upper character!");
        return FALSE;
    }
    if(!lower_included){
        print_error_message(w, height, width, y, x, "Password must have at least 1 lower character!");
        return FALSE;
    }
    if(!int_included){
        print_error_message(w, height, width, y, x, "Password must have at least 1 digit character!");
        return FALSE;
    }
    return TRUE;
}

bool email_validated(WINDOW* w, int height, int width, int y, int x, char* email){
    regex_t regex;
    int ret;

    ret = regcomp(&regex, "^[a-zA-Z0-9._%%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]+$", REG_EXTENDED);
    
    ret = regexec(&regex, email, 0, NULL, 0);
    regfree(&regex);

    if (!ret) {
        return TRUE;
    } else if (ret == REG_NOMATCH) {
        print_error_message(w, height, width, y, x, "Enter a correct Email! like xxx@yyy.ccc !");
        return FALSE;
    }
}

// Signup Form Handeling
void get_username(WINDOW* sign_form, int height, int width, int y, int x, char* username){
    int user_ch;
    int index = 0;

    clear_part(sign_form, y, x, y, width - 2);
    wmove(sign_form, y, x);
    curs_set(TRUE);
    noecho();

    while(1){
        user_ch = wgetch(sign_form);

        if(user_ch == '\n'){
            username[index] = '\0';

            if(does_user_exist(username)){
                print_error_message(sign_form, height, width, y, x+index, "This Username has already taken!");
            } else if(index == 0){
                print_error_message(sign_form, height, width, y, x+index, "Username box must be completed!");
            } else{
                curs_set(FALSE);
                break;
            }
            continue;
        } else if(user_ch == KEY_BACKSPACE || user_ch == 127) {
            if (index > 0) {
                index--;
                username[index] = '\0';
                mvwaddch(sign_form, y, x+index, ' ');
                wmove(sign_form, y, x+index);
            }
            continue;
        } else if(user_ch < 33 || user_ch > 126) {
            continue;
        }

        username[index] = user_ch;
        mvwaddch(sign_form, y, x+index, user_ch);
        wrefresh(sign_form);
        index++;
    }
    clear_part(sign_form, height-2, 1, height-2, width - 2);
}

void get_password(WINDOW* sign_form, int height, int width, int y, int x, char* password){
    int pass_ch;
    int index = 0;
    
    clear_part(sign_form, y, x, y, width - 2);
    print_error_message(sign_form, height, width, y, x, "if you want to generate random pass press ENTER!");

    wmove(sign_form ,y, x);
    curs_set(TRUE);
    keypad(sign_form, TRUE);
    noecho();

    while(1){
        pass_ch = wgetch(sign_form);

        if(pass_ch == '\n'){
            if(index == 0){
                break;
            }
            if(password_validated(sign_form, height, width, y, x+index, password)){
                break;
            } else{
                continue;
            }
        } else if(pass_ch == KEY_BACKSPACE || pass_ch == 127) {
            if (index > 0) {
                index--;
                password[index] = '\0';
                mvwaddch(sign_form, y, x+index, ' ');
                wmove(sign_form, y, x+index);
            }
            continue;
        } else if(pass_ch < 33 || pass_ch > 126) {
            continue;
        }

        if(index >= MAX_PASSWORD){
            print_error_message(sign_form, height, width, y, x+index, "password must be maximum 20 characters!");
            continue;
        }
        password[index] = pass_ch;
        mvwaddch(sign_form, y, x+index, '*');
        wrefresh(sign_form);
        index++;
    }
    clear_part(sign_form, height-2, 1, height-2, width - 2);
    password[index] = '\0';
    wrefresh(sign_form);
}

void get_email(WINDOW* sign_form, int height, int width, int y, int x, char* email){
    int email_ch;
    int index = 0;

    wmove(sign_form, y, x);
    curs_set(TRUE);
    keypad(sign_form, TRUE);
    echo();

    while(1){
        email_ch =  wgetch(sign_form);

        if(email_ch == '\n'){
            if(email_validated(sign_form, height, width, y, x+index, email))
                break;
            else
                continue;
        } else if(email_ch == KEY_BACKSPACE || email_ch == 127) {
            if (index > 0) {
                index--;
                email[index] = '\0';
                mvwaddch(sign_form, y, x+index, ' ');
                wmove(sign_form, y, x+index);
            }
            continue;
        } else if(email_ch == KEY_LEFT || email_ch == KEY_RIGHT) {
            continue;
        }
        if(index >= MAX_EMAIL){
            print_error_message(sign_form, height, width, y, x+index, "Email must be maximum 25 characters!");
            continue;
        }

        email[index] = email_ch;
        index++;
    }
    email[index] = '\0';
    clear_part(sign_form, height-2, 1, height-2, width - 2);
    wrefresh(sign_form);
}

void get_checker_word(WINDOW* sign_form, int height, int width, int y, int x, char* checker_w){
    int check_ch;
    int index = 0;

    wmove(sign_form, y, x);
    curs_set(TRUE);
    keypad(sign_form, TRUE);
    echo();

    while(1){
        check_ch = wgetch(sign_form);
        if(check_ch == '\n'){
            break;
        } else if(check_ch == KEY_BACKSPACE || check_ch == 127) {
            if (index > 0) {
                index--;
                checker_w[index] = '\0';
                mvwaddch(sign_form, y, x+index, ' ');
                wmove(sign_form, y, x+index);
            }
            continue;
        } else if(check_ch == KEY_LEFT || check_ch == KEY_RIGHT) {
            continue;
        }
        if(index >= MAX_USERNAME){
            print_error_message(sign_form, height, width, y, x+index, "Checker word must be maximum 20 characters!");
            continue;
        }

        checker_w[index] = check_ch;
        index++;

    }

    clear_part(sign_form, height-2, 1, height-2, width - 2);

    checker_w[index] = '\0';
    curs_set(FALSE);
    noecho();
}

void generate_random_pass(WINDOW* sign_form, int height, int width, int y_pass, int x_pass, char* pass){
    clear_part(sign_form, height-2, 1, height-2, width - 2);
    char* ms = "Do you want to generate a random pass? (y/n)";
    print_error_message(sign_form, height, width, height - 2, width/2 + strlen(ms)/2 + 1, ms);
    
    curs_set(TRUE);
    int c = wgetch(sign_form);
    curs_set(FALSE);

    if(c == 'y'){
        int len = rand_in(7, MAX_PASSWORD);
        for(int i = 0; i < len; i++){
            pass[i] = ' ';
        }
        pass[len] = '\0';

        int num_upper = rand_in(2, len/4);
        int num_dig = rand_in(2, len/4);

        int upper_indexs[num_upper], dig_indexs[num_dig];
        rand_ints(upper_indexs, num_upper, 0, len, 1);
        rand_ints(dig_indexs, num_dig, 0, len, 1);
        
        for(int i = 0; i<num_upper; i++){
            int l = rand_in('A', 'Z' + 1);
            pass[upper_indexs[i]] = l;
        }
        for(int i = 0; i<num_dig; i++){
            int l = rand_in('0', '9' + 1);
            pass[dig_indexs[i]] = l;
        }
        
        for(int i = 0; i < len; i++){
            if(pass[i] == ' '){
                pass[i] = rand_in('a', 'z' + 1);
            }
        }
        char ms[50];
        sprintf(ms, "%s %s", "Now your password is: ", pass);
        print_error_message(sign_form, height, width, height - 2, width/2 + strlen(ms)/2 + 1, ms);
        clear_part(sign_form, y_pass, x_pass, y_pass, width - 2);
        for(int i = 0; i < len; i++){
            mvwaddch(sign_form, y_pass, x_pass+i, '*');
        }
        int c = getchar();
        clear_part(sign_form, height-2, 1, height-2, width - 2);
        refresh();
    }

    clear_part(sign_form, height-2, 1, height-2, width - 2);
    wrefresh(sign_form);
}

void save_user_data(User user){
    
}

void signup_user(){
    int width = 50, height = 26;
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
    
    int y_start = 6;
    int x_start = 18;
    
    int selected = -1;
    int pressed = 0;
    print_buttons(sign_form, signup_form_buttons, 2, selected, y_start + 10, width/2);
    print_messages(sign_form, signup_form_labels, 4, y_start, x_start, 'r', LABEL_COLOR);

    User user;

    user.username = (char *)malloc((MAX_USERNAME + 5) * sizeof(char));
    get_username(sign_form, height, width, y_start, x_start, user.username);

    user.password = (char *)malloc((MAX_PASSWORD + 20) * sizeof(char));
    get_password(sign_form, height, width, y_start + 2, x_start, user.password);

    user.email = (char *)malloc((MAX_EMAIL) * sizeof(char));
    get_email(sign_form, height, width, y_start + 4, x_start, user.email);

    user.checker_w = (char *)malloc((MAX_USERNAME + 5) * sizeof(char));
    get_checker_word(sign_form, height, width, y_start + 6, x_start, user.checker_w);
    
    selected = 0;
    while(pressed == 0){
        print_buttons(sign_form, signup_form_buttons, 2, selected, y_start + 10, width/2);
        handle_selected_btn(&selected, 2, &pressed);
        
        if(pressed == 1){
            if(selected == 0)
                generate_random_pass(sign_form, height, width, y_start+2, x_start, user.password);
            else{
                save_user_data(user);
                break;
            }
            pressed = 0;
        } else if(pressed == -1){
            endwin();
            exit(0);
        }
        refresh();
        wrefresh(sign_form);
    }

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
        print_messages(menu, login_messages, message_num, 2, width/2, 'c', HEADER_COLOR);
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