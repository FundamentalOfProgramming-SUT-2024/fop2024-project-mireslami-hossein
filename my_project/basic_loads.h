#ifndef BASIC_LOADS_H
#define BASIC_LOADS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <time.h>
#include "cjson/cJSON.h"

// MAX_SIZES
#define MAX_USERNAME 15
#define MAX_PASSWORD 20
#define MAX_EMAIL 25

// New COLORS
#define COLOR_PURPLE 10
#define CUSTOM_RED 11
#define CUSTOM_CYAN 12
#define CUSTOM_GREEN 13
#define CUSTOM_ORANGE 14
#define CUSTOM_YELLOW 15

// New PAIRS
#define BG_LOGIN 0
#define MESSAGE_COLOR 1
#define BTN_DEFAULT 2
#define BTN_SELECTED 3
#define HEADER_COLOR 4
#define LABEL_COLOR 5
#define TEXT_COLOR 6

#define BTN_DEFAULT_2 7
#define BTN_SELECTED_2 8

// Structs
typedef struct{

} Game;

typedef struct{
    int x,y;
} Loc;

typedef struct {
    char* username;
    char* password;
    char* email;
    char* checker_w;
    bool is_guest;

    int rank;
    int points;
    int golds;
    int ended_games;
    
    // Time to first Game
    time_t experience;
    
    int last_game_id;

} User;

// graphics
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

void print_title(WINDOW* w, char* title, int y, int x){
    wattron(w, A_BOLD | COLOR_PAIR(HEADER_COLOR));
    mvwprintw(w, y, x - strlen(title)/2, "%s", title);
    wattroff(w, A_BOLD | COLOR_PAIR(HEADER_COLOR));
    wrefresh(w);
}

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

void print_buttons(WINDOW* w, char btns[][50], int size, int selected, int y, int x, int color_mode){
    wattron(w, A_BOLD);
    
    for(int i = 0; i < size; i ++){
        int btn_len = strlen(btns[i]);
        int x_m = x - btn_len/2;
        int y_m = y + i*2 + 1;
        if(i == selected){
            if(color_mode == 1){
                wattron(w, COLOR_PAIR(BTN_SELECTED));
                mvwprintw(w,y_m, x_m, "%s", btns[i]);
                wattroff(w, COLOR_PAIR(BTN_SELECTED));
            } else if(color_mode == 2){
                wattron(w, COLOR_PAIR(BTN_SELECTED_2));
                mvwprintw(w,y_m, x_m, "%s", btns[i]);
                wattroff(w, COLOR_PAIR(BTN_SELECTED_2));
            } 
        } else{
            if(color_mode == 1){
                wattron(w, COLOR_PAIR(BTN_DEFAULT));
                mvwprintw(w,y_m, x_m, "%s", btns[i]);
                wattroff(w, COLOR_PAIR(BTN_DEFAULT));
            } else if(color_mode == 2){
                wattron(w, COLOR_PAIR(BTN_DEFAULT_2));
                mvwprintw(w,y_m, x_m, "%s", btns[i]);
                wattroff(w, COLOR_PAIR(BTN_DEFAULT_2));
            }
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
    
    // on press Enter
    case 10: case 13:
        *flag = 1;
        break;
    
    case 'q':case 'Q':
        *flag = -1;
        break;
    }
}


// randoms
int rand_in(int a, int b){
    if(a == b) return a;
    return (rand() % (b-a) + a);
}

void rand_ints(int* arr, int size, int a, int b, int ascending){
    for(int i = 0; i < size; i++){
        int x = rand_in(a, b);
        bool used = FALSE;
        for(int j = 0; j < i; j ++){
            if(arr[j] == x){
                used = TRUE;
                break;
            }
        }

        if(!used){
            arr[i] = x;
        } else{
            i--;
        }
    }
    if(ascending){
        for(int i = 0; i < size; i++){
            for(int j = 0; j < size; j++){
                if(arr[i] < arr[j]){
                    int t = arr[i];
                    arr[i] = arr[j];
                    arr[j] = t;
                }
            }
        }
    }
}

// string functions
bool is_upper(char c){
    return (c >= 'A' && c <= 'Z');
} 
bool is_lower(char c){
    return (c >= 'a' && c <= 'z');
}
bool is_digit(char c){
    return (c >= '0' && c <= '9');
}

// Files
char* read_file(char* path){
    FILE* fptr = fopen(path, "r");

    fseek(fptr, 0, SEEK_END);
    long file_size = ftell(fptr);
    rewind(fptr);

    char* data = (char *)malloc(file_size + 1);
    fread(data, 1, file_size, fptr);

    fclose(fptr);

    return data;
}

int write_file(char* path, char* data){
    FILE* fptr = fopen(path, "w");
    if(!fptr) return -1;
    fprintf(fptr, "%s", data);
    fclose(fptr);
    return 0;
}

int count_users(){
    char* users_data = read_file("data/users.json");
    cJSON* root = cJSON_Parse(users_data);
    if(!root){
        return 0;
    }

    cJSON* user_number = cJSON_GetObjectItem(root, "users_number");
    return user_number->valueint;
}

// User
void reset_user_data(User* user){
    user->username = (char *)malloc((MAX_USERNAME + 5) * sizeof(char));
    user->password = (char *)malloc((MAX_PASSWORD + 5) * sizeof(char));
    user->email = (char *)malloc((MAX_EMAIL) * sizeof(char));
    user->checker_w = (char *)malloc((MAX_USERNAME + 5) * sizeof(char));

    user->points = 0;
    user->rank = 0;
    user->golds = 0;
    user->ended_games = 0;

    user->last_game_id = -1;
    user->experience = 0;
}

void get_user_detail_by_username(char* target_username, char* data, char* value){
    char* users_data = read_file("data/users.json");
    cJSON* root = cJSON_Parse(users_data);
    if(!root){
        return;
    }

    cJSON* users = cJSON_GetObjectItem(root, "users");
    int c = count_users();
    for(int i = 0; i < c; i++){
        cJSON* user = cJSON_GetArrayItem(users, i);
        cJSON* username = cJSON_GetObjectItem(user, "username");
        if(strcmp(username->valuestring, target_username) == 0){
            cJSON* detail = cJSON_GetObjectItem(user, data);
            strcpy(value, detail->valuestring);
            cJSON_Delete(root);
            return;
        }
    }
}

void save_user_data(User user){
    char* users_data = read_file("data/users.json");

    cJSON* root = cJSON_Parse(users_data);
    if(!root){
        root = cJSON_CreateObject();
        cJSON_AddNumberToObject(root, "users_number", 0);
        cJSON* users = cJSON_CreateArray();
        cJSON_AddItemToObject(root, "users", users);
    }
    
    cJSON* users = cJSON_GetObjectItem(root, "users");
    
    cJSON* user_number = cJSON_GetObjectItem(root, "users_number");

    cJSON_SetIntValue(user_number, user_number->valueint+1);
    cJSON *user_data = cJSON_CreateObject();
    cJSON_AddItemToObject(user_data, "username", cJSON_CreateString(user.username));
    cJSON_AddItemToObject(user_data, "password", cJSON_CreateString(user.password));
    cJSON_AddItemToObject(user_data, "email", cJSON_CreateString(user.email));
    cJSON_AddItemToObject(user_data, "checker_word", cJSON_CreateString(user.checker_w));
    
    cJSON_AddItemToObject(user_data, "rank", cJSON_CreateNumber(user.rank));
    cJSON_AddItemToObject(user_data, "points", cJSON_CreateNumber(user.points));
    cJSON_AddItemToObject(user_data, "golds", cJSON_CreateNumber(user.golds));
    cJSON_AddItemToObject(user_data, "ended_games", cJSON_CreateNumber(user.ended_games));
    
    cJSON_AddItemToObject(user_data, "lastgame_id", cJSON_CreateNumber(user.last_game_id));
    cJSON_AddItemToObject(user_data, "experience", cJSON_CreateNumber(user.experience));

    cJSON_AddItemToArray(users, user_data);

    char* file_data = cJSON_Print(root);
    write_file("data/users.json", file_data);
}

void game_initalize(){
    start_color();
    add_color_rgb(COLOR_PURPLE, 71, 50, 122);
    add_color_rgb(CUSTOM_RED, 203, 35, 44);
    add_color_rgb(CUSTOM_GREEN, 93, 255, 101);
    add_color_rgb(CUSTOM_CYAN, 151, 176, 243);
    add_color_rgb(CUSTOM_ORANGE, 255, 178, 131);
    add_color_rgb(CUSTOM_YELLOW, 245, 255, 155);

    init_pair(MESSAGE_COLOR, COLOR_RED, COLOR_BLACK);
    init_pair(BTN_DEFAULT, COLOR_CYAN, COLOR_BLACK);
    init_pair(BTN_SELECTED,COLOR_BLACK , COLOR_CYAN);
    init_pair(BG_LOGIN, COLOR_PURPLE, COLOR_PURPLE);
    init_pair(HEADER_COLOR, CUSTOM_CYAN, COLOR_BLACK);
    init_pair(TEXT_COLOR, CUSTOM_YELLOW, COLOR_BLACK);
    init_pair(LABEL_COLOR, CUSTOM_ORANGE, COLOR_BLACK);
    init_pair(BTN_DEFAULT_2, CUSTOM_GREEN, COLOR_BLACK);
    init_pair(BTN_SELECTED_2, COLOR_BLACK , CUSTOM_GREEN);
    srand(time(NULL));
}

#endif