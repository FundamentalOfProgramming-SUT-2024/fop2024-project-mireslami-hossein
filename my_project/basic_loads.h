#ifndef BASIC_LOADS_H
#define BASIC_LOADS_H

#include <stdio.h>
#include <stdlib.h>
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

// New PAIRS
#define BG_LOGIN 0
#define MESSAGE_COLOR 1
#define BTN_DEFAULT 2
#define BTN_SELECTED 3
#define HEADER_COLOR 4
#define TEXT_COLOR 5
#define LABEL_COLOR 5

// Structs
typedef struct{

} Game;

typedef struct{

} Game_time;

typedef struct {
    char* username;
    char* password;
    char* email;
    char* checker_w;

    int points;
    int golds;
    int grade;
    
    Game_time exprience;
    Game* last_game;

    int ended_games;
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

// // user files
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
    user->grade = 0;
    user->golds = 0;
    user->ended_games = 0;

    user->last_game = (Game *)malloc(sizeof(Game));
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

    srand(time(NULL));
}

#endif