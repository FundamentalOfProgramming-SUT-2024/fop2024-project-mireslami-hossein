#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include "cjson/cJSON.h"
#include <regex.h>
#include <time.h>

#include "basic_loads.h"


char entry_messages[3][50] = {
    "Welcome!", "Choose a way to continue:", "(Q to exit)"
};
char entry_options[3][50] = {
    "Sign Up", "Login", "Login as Guest"
};

char signup_form_labels[4][50] = {
    "Username*: ", "Password*: ", "Email*: ", "Checker Word: "
};
char signup_form_buttons[2][50] = {
    "Generate Random Password", "Submit"
};

// Login by user pass
char login_form_labels[2][50] = {
    "Username: ", "Password: "
};
char login_form_buttons[1][50] = {
    "Submit"
};
// Password Recovery
char pass_form_labels[4][50] = {
    "Username: ", "Email: ", "Checker Word: ", "New password: "
};


// Signup Validation
void print_error_message(WINDOW* w, int height, int width,  int reset_y, int reset_x, char* ms){
    clear_part(w, height-2, 1, height-2, width - 2);
    wattron(w, COLOR_PAIR(MESSAGE_COLOR) | A_BOLD);
    mvwprintw(w, height - 2 , width/2 - strlen(ms)/2, "%s", ms);
    wattroff(w, COLOR_PAIR(MESSAGE_COLOR)| A_BOLD);
    wrefresh(w);
    wmove(w,reset_y, reset_x);
}

int count_users();
void read_usernames(char** usernames, int number_of_users);
void generate_random_pass(WINDOW* sign_form, int height, int width, int y_pass, int x_pass, char* pass);

bool does_user_exist(char* name){
    bool result = FALSE;
    int number_of_users = count_users();
    if(number_of_users == 0) return result;
    char** usernames = (char**)malloc(number_of_users * sizeof(char*));
    for(int i = 0; i < number_of_users; ++i){
        usernames[i] = (char*)malloc(MAX_USERNAME* sizeof(char));
    }
    read_usernames(usernames, number_of_users);

    for(int i = 0; i < number_of_users; i++){
        if(strcmp(name, usernames[i]) == 0){
            result = TRUE;
            break;
        }
    }

    for(int i = 0; i < number_of_users; i++){
        free(usernames[i]);
    }
    free(usernames);

    return result;
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
void get_sign_username(WINDOW* sign_form, int height, int width, int y, int x, char* username){
    int user_ch;
    int index = 0;

    clear_part(sign_form, y, x, y, width - 2);
    wmove(sign_form, y, x);
    keypad(sign_form, TRUE);
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
        } else if(user_ch < 32 || user_ch > 126) {
            continue;
        }
        if(index >= MAX_USERNAME){
            char ms[50];
            sprintf(ms, "username must be maximum %d characters!", MAX_USERNAME);
            print_error_message(sign_form, height, width, y, x+index, ms);
            continue;
        }
        username[index] = user_ch;
        mvwaddch(sign_form, y, x+index, user_ch);
        wrefresh(sign_form);
        index++;
    }
    clear_part(sign_form, height-2, 1, height-2, width - 2);
}

void get_sign_password(WINDOW* sign_form, int height, int width, int y, int x, char* password){
    int pass_ch;
    int index = 0;
    
    clear_part(sign_form, y, x, y, width - 2);
    print_error_message(sign_form, height, width, y, x, "If you want to generate random pass press ENTER!");

    wmove(sign_form ,y, x);
    curs_set(TRUE);
    keypad(sign_form, TRUE);
    noecho();

    while(1){
        pass_ch = wgetch(sign_form);
        clear_part(sign_form, height-2, 1, height-2, width - 2);
        if(pass_ch == '\n'){
            if(index == 0){
                generate_random_pass(sign_form, height, width, y, x, password);
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
        } else if(pass_ch < 32 || pass_ch > 126) {
            continue;
        }

        if(index >= MAX_PASSWORD){
            char ms[50];
            sprintf(ms, "password must be maximum %d characters!", MAX_PASSWORD);
            print_error_message(sign_form, height, width, y, x+index, ms);
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

void get_sign_email(WINDOW* sign_form, int height, int width, int y, int x, char* email){
    int email_ch;
    int index = 0;

    wmove(sign_form, y, x);
    curs_set(TRUE);
    keypad(sign_form, TRUE);
    noecho();

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
        } else if(email_ch < 32 || email_ch > 126) {
            continue;
        }
        if(index >= MAX_EMAIL){
            char ms[50];
            sprintf(ms, "Email must be maximum %d characters!", MAX_EMAIL);
            print_error_message(sign_form, height, width, y, x+index, ms);            continue;
        }
        mvwaddch(sign_form, y, x+index, email_ch);
        email[index] = email_ch;
        index++;
    }

    email[index] = '\0';
    clear_part(sign_form, height-2, 1, height-2, width - 2);
    wrefresh(sign_form);
}

void get_sign_checker_word(WINDOW* sign_form, int height, int width, int y, int x, char* checker_w){
    int check_ch;
    int index = 0;

    wmove(sign_form, y, x);
    curs_set(TRUE);
    keypad(sign_form, TRUE);
    noecho();

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
        } else if(check_ch < 32 || check_ch > 126) {
            continue;
        }
        if(index >= MAX_USERNAME){
            char ms[50];
            sprintf(ms, "checker word must be maximum %d characters!", MAX_USERNAME);
            print_error_message(sign_form, height, width, y, x+index, ms);
            continue;
        }

        checker_w[index] = check_ch;
        mvwaddch(sign_form, y, x+index, check_ch);
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


// Login Pass Forgotten
void get_forgot_email(WINDOW* forgot_pass_win, int height, int width, int y, int x, User* user){
    int email_ch;
    int index = 0;

    wmove(forgot_pass_win, y, x);
    curs_set(TRUE);
    keypad(forgot_pass_win, TRUE);
    noecho();

    while(1){
        email_ch =  wgetch(forgot_pass_win);

        if(email_ch == '\n'){
            user->email[index] = '\0';
            char target_email[MAX_EMAIL + 1];
            get_user_detail_by_username(user->username, "email", target_email);
            if(strcmp(target_email, user->email)){
                print_error_message(forgot_pass_win, height, width, y, x+index, "Email isn't correct!");
                continue;
            } else{
                break;
            }
        } else if(email_ch == KEY_BACKSPACE || email_ch == 127) {
            if (index > 0) {
                index--;
                user->email[index] = '\0';
                mvwaddch(forgot_pass_win, y, x+index, ' ');
                wmove(forgot_pass_win, y, x+index);
            }
            continue;
        } else if(email_ch < 32 || email_ch > 126) {
            continue;
        }
        if(index >= MAX_EMAIL){
            continue;
        }
        mvwaddch(forgot_pass_win, y, x+index, email_ch);
        user->email[index] = email_ch;
        index++;
    }

    user->email[index] = '\0';
    clear_part(forgot_pass_win, height-2, 1, height-2, width - 2);
    wrefresh(forgot_pass_win);
}

void get_forgot_checker_word(WINDOW* forgot_pass_win, int height, int width, int y, int x, User* user){
    int ch;
    int index = 0;

    wmove(forgot_pass_win, y, x);
    curs_set(TRUE);
    keypad(forgot_pass_win, TRUE);
    noecho();

    while(1){
        ch =  wgetch(forgot_pass_win);

        if(ch == '\n'){
            user->checker_w[index] = '\0';
            char target_checkw[MAX_USERNAME + 1];
            get_user_detail_by_username(user->username, "checker_word", target_checkw);
            if(strcmp(target_checkw, user->checker_w)){
                print_error_message(forgot_pass_win, height, width, y, x+index, "Checker Word isn't correct!");
                continue;
            } else{
                break;
            }
        } else if(ch == KEY_BACKSPACE || ch == 127) {
            if (index > 0) {
                index--;
                user->checker_w[index] = '\0';
                mvwaddch(forgot_pass_win, y, x+index, ' ');
                wmove(forgot_pass_win, y, x+index);
            }
            continue;
        } else if(ch < 32 || ch > 126) {
            continue;
        }
        if(index >= MAX_USERNAME){
            continue;
        }
        mvwaddch(forgot_pass_win, y, x+index, ch);
        user->checker_w[index] = ch;
        index++;
    }

    user->checker_w[index] = '\0';
    clear_part(forgot_pass_win, height-2, 1, height-2, width - 2);
    wrefresh(forgot_pass_win);
}

void update_user_pass(User t_user);
void get_forgot_new_password(WINDOW* forgot_pass_win, int height, int width, int y, int x, User* user){
    int ch;
    char password[MAX_PASSWORD + 1];
    int index = 0;

    wmove(forgot_pass_win, y, x);
    curs_set(TRUE);
    keypad(forgot_pass_win, TRUE);
    noecho();

    while(1){
        ch =  wgetch(forgot_pass_win);

        if(ch == '\n'){
            password[index] = '\0';
            if(password_validated(forgot_pass_win, height, width, y, x+index, password)){
                break;
            } else{
                continue;
            }

        } else if(ch == KEY_BACKSPACE || ch == 127) {
            if (index > 0) {
                index--;
                password[index] = '\0';
                mvwaddch(forgot_pass_win, y, x+index, ' ');
                wmove(forgot_pass_win, y, x+index);
            }
            continue;
        } else if(ch < 32 || ch > 126) {
            continue;
        }
        if(index >= MAX_USERNAME){
            continue;
        }
        mvwaddch(forgot_pass_win, y, x+index, '*');
        password[index] = ch;

        index++;
    }
    password[index] = '\0';
    strcpy(user->password, password);
    update_user_pass(*user);
    clear_part(forgot_pass_win, height-2, 1, height-2, width - 2);
}

void login_user(User* user);
void password_forget_panel(int height, int width, int y_pass, int x_pass, User* user){
    clear();
    int y = LINES/2 - height/2;
    int x = COLS/2 - width/2;
    WINDOW* forgot_pass_win = newwin(height, width, y, x);
    box(forgot_pass_win, 0, 0);
    refresh();

    char title[20] = "Password Recovery";
    start_color();
    wattron(forgot_pass_win, COLOR_PAIR(HEADER_COLOR) | A_BOLD);
    mvwprintw(forgot_pass_win,3,width/2 - strlen(title)/2, "%s",title);
    wattroff(forgot_pass_win, COLOR_PAIR(HEADER_COLOR)| A_BOLD);

    int selected = -1;
    int pressed = 0;
    int y_start = 6;
    int x_start = 17;

    print_messages(forgot_pass_win, pass_form_labels, 3, y_start, x_start, 'r', LABEL_COLOR);
    print_buttons(forgot_pass_win, login_form_buttons, 1, selected, y_start + 12, width/2, 1);

    mvwprintw(forgot_pass_win, y_start, x_start, "%s", user->username);
    wrefresh(forgot_pass_win);

    get_forgot_email(forgot_pass_win, height, width, y_start + 2, x_start, user);

    get_forgot_checker_word(forgot_pass_win, height, width, y_start + 4, x_start, user);

    print_messages(forgot_pass_win, pass_form_labels, 4, y_start, x_start, 'r', LABEL_COLOR);

    get_forgot_new_password(forgot_pass_win, height, width, y_start + 6, x_start, user);
    
    curs_set(FALSE);

    selected = 0;
    while(pressed == 0){
        print_buttons(forgot_pass_win, login_form_buttons, 1, selected, y_start + 12, width/2, 1);
        handle_selected_btn(&selected, 1, &pressed);
        if(pressed == -1){
            endwin();
            exit(0);
        }
        else if(pressed == 1){
            clear();
            delwin(forgot_pass_win);
            login_user(user);
        }
    }
}

// Login Form Handeling
void get_login_username(WINDOW* login_form, int height, int width, int y, int x, char* username){
    int user_ch;
    int index = 0;

    clear_part(login_form, y, x, y, width - 2);
    wmove(login_form, y, x);
    curs_set(TRUE);
    keypad(login_form, TRUE);
    noecho();

    while(1){
        user_ch = wgetch(login_form);

        if(user_ch == '\n'){
            username[index] = '\0';

            if(!does_user_exist(username)){
                print_error_message(login_form, height, width, y, x+index, "This Username does not exist!");
                continue;
            } else{
                break;
            }
        } else if(user_ch == KEY_BACKSPACE || user_ch == 127) {
            if (index > 0) {
                index--;
                username[index] = '\0';
                mvwaddch(login_form, y, x+index, ' ');
                wmove(login_form, y, x+index);
            }
            continue;
        } else if(user_ch < 32 || user_ch > 126) {
            continue;
        }
        if(index >= MAX_USERNAME){
            continue;
        }
        username[index] = user_ch;
        mvwaddch(login_form, y, x+index, user_ch);
        wrefresh(login_form);
        index++;
    }
    clear_part(login_form, height-2, 1, height-2, width - 2);
}

void get_login_password(WINDOW* login_form, int height, int width, int y, int x, User* user){
    int pass_ch;
    int index = 0;
    bool password_reset = false;

    clear_part(login_form, y, x, y, width - 2);

    wmove(login_form ,y, x);
    curs_set(TRUE);
    keypad(login_form, TRUE);
    noecho();

    print_error_message(login_form, height, width, y, x+index, "If you forgot your pass press ENTER!");
    while(1){
        pass_ch = wgetch(login_form);
        clear_part(login_form, height-2, 1, height-2, width - 2);
        if(index == 0) print_error_message(login_form, height, width, y, x+index, "If you forgot your pass press ENTER!");
        if(pass_ch == '\n' && index == 0 && !password_reset){
            password_reset = true;
            break;
        }
        if(pass_ch == '\n'){
            user->password[index] = '\0';
            char target_pass[MAX_PASSWORD + 1];
            get_user_detail_by_username(user->username, "password", target_pass);
            if(strcmp(target_pass, user->password)){
                print_error_message(login_form, height, width, y, x+index, "Password isn't correct!");
                continue;
            } else{
                break;
            }
        } else if(pass_ch == KEY_BACKSPACE || pass_ch == 127) {
            if (index > 0) {
                index--;
                user->password[index] = '\0';
                mvwaddch(login_form, y, x+index, ' ');
                wmove(login_form, y, x+index);
            }
            continue;
        } else if(pass_ch < 32 || pass_ch > 126) {
            continue;
        }

        if(index >= MAX_PASSWORD){
            continue;
        }
        user->password[index] = pass_ch;
        mvwaddch(login_form, y, x+index, '*');
        wrefresh(login_form);
        index++;
    }
    clear_part(login_form, height-2, 1, height-2, width - 2);
    if(password_reset){
        password_forget_panel(height, width + 5, y, x, user);
    }
}

// Data
void read_usernames(char** usernames, int number_of_users){
    char* users_data = read_file("data/users.json");

    cJSON* root = cJSON_Parse(users_data);
    if(!root){
        return;
    }

    cJSON* users = cJSON_GetObjectItem(root, "users");
    
    for(int i = 0; i < number_of_users; ++i){
        cJSON* user = cJSON_GetArrayItem(users, i);
        cJSON* username = cJSON_GetObjectItem(user, "username");
        strcpy(usernames[i], username->valuestring);
    }
    cJSON_Delete(root);
    free(users_data);
}

void update_user_pass(User t_user){
    char* users_data = read_file("data/users.json");

    cJSON* root = cJSON_Parse(users_data);
    cJSON* users = cJSON_GetObjectItem(root, "users");
    cJSON* user_number = cJSON_GetObjectItem(root, "users_number");
    for(int i = 0; i < user_number->valueint; i++){
        cJSON* user = cJSON_GetArrayItem(users, i);
        cJSON* username = cJSON_GetObjectItem(user, "username");

        if(strcmp(username->valuestring, t_user.username) == 0){
            cJSON* password = cJSON_GetObjectItem(user, "password");
            cJSON_SetValuestring(password, t_user.password);
            break;
        }
    }

    char* file_data = cJSON_Print(root);
    write_file("data/users.json", file_data);
}

void login_as_guest(User* user){
    user->is_guest = true;
}

void login_user(User* user){
    int width = 40, height = 25;
    int y = LINES/2 - height/2;
    int x = COLS/2 - width/2;
    WINDOW* login_form = newwin(height, width, y, x);
    box(login_form, 0, 0);
    refresh();

    char title[20] = "Login";
    start_color();
    wattron(login_form, COLOR_PAIR(HEADER_COLOR) | A_BOLD);
    mvwprintw(login_form,3,width/2 - strlen(title)/2, "%s",title);
    wattroff(login_form, COLOR_PAIR(HEADER_COLOR)| A_BOLD);

    int y_start = 6;
    int x_start = 15;
    
    int selected = -1;
    int pressed = 0;

    print_messages(login_form, login_form_labels, 4, y_start, x_start, 'r', LABEL_COLOR);
    print_buttons(login_form, login_form_buttons, 1, selected, y_start + 10, width/2, 1);

    get_login_username(login_form, height, width, y_start, x_start, user->username);
    get_login_password(login_form, height, width, y_start + 2, x_start, user);

    user->is_guest = FALSE;

    curs_set(FALSE);
    selected = 0;
    while(pressed == 0){
        print_buttons(login_form, login_form_buttons, 1, selected, y_start + 10, width/2, 1);
        handle_selected_btn(&selected, 1, &pressed);
        if(pressed == -1){
            endwin();
            exit(0);
        }
        else if(pressed == 1){
            // Open Game Page
            break;
        }
    }
    delwin(login_form);
}

void signup_user(User* user){
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
    print_buttons(sign_form, signup_form_buttons, 2, selected, y_start + 10, width/2, 1);
    print_messages(sign_form, signup_form_labels, 4, y_start, x_start, 'r', LABEL_COLOR);


    get_sign_username(sign_form, height, width, y_start, x_start, user->username);

    get_sign_password(sign_form, height, width, y_start + 2, x_start, user->password);

    get_sign_email(sign_form, height, width, y_start + 4, x_start, user->email);

    get_sign_checker_word(sign_form, height, width, y_start + 6, x_start, user->checker_w);
    
    user->is_guest = FALSE;

    selected = 0;
    while(pressed == 0){
        print_buttons(sign_form, signup_form_buttons, 2, selected, y_start + 10, width/2, 1);
        handle_selected_btn(&selected, 2, &pressed);
        
        if(pressed == 1){
            if(selected == 0)
                generate_random_pass(sign_form, height, width, y_start+2, x_start, user->password);
            else if(selected == 1){
                // Open Game Page
                save_user_data(*user);
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

    delwin(sign_form);
}


void open_form(int selected, User* user){
    clear();
    // Sign Up
    if(selected == 0){
        signup_user(user);
    } else if(selected == 1){
        login_user(user);
    } else if(selected == 2){
        login_as_guest(user);
    }
    refresh();
}

void load_first_page(User* user){
    clear();

    start_color();
    game_initalize();

    int selected_btn = 0;
    int exit_flag = 0;
    int btn_nums = 3;
    int message_num = 3;

    int height = 18, width = 35;
    int y = LINES/2 - height/2;
    int x = COLS/2 - width/2;
    WINDOW* menu = newwin(height , width,y,x);
    wrefresh(menu);
    box(menu, 0, 0);
    refresh();

    
    while(!exit_flag){
        print_messages(menu, entry_messages, message_num, 2, width/2, 'c', HEADER_COLOR);
        print_buttons(menu, entry_options, btn_nums, selected_btn, 9, width/2, 1);
        
        handle_selected_btn(&selected_btn, btn_nums, &exit_flag);
    }

    if(exit_flag == -1){
        endwin();
        exit(0);
    }

    delwin(menu);
    open_form(selected_btn, user);
}