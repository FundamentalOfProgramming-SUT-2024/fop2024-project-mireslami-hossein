#ifndef BASIC_LOADS_H
#define BASIC_LOADS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <time.h>
#include "cjson/cJSON.h"
#include <locale.h>

// New KEYs
#define KEY_ESC 27

// MAX_SIZES
#define MAX_USERNAME 15
#define MAX_PASSWORD 20
#define MAX_EMAIL 25

#define MAX_OBJ_ROOM 10
#define MAX_WEAPON_ROOM 10
#define MAX_DOORS_ROOM 3
#define MAX_ENEMY_LEVEL 30

// New COLORS
enum CustomColors {
    COLOR_PURPLE = 30,
    CUSTOM_RED,
    CUSTOM_CYAN,
    CUSTOM_GREEN,
    CUSTOM_ORANGE,
    CUSTOM_ORANGE_2,
    CUSTOM_ORANGE_3,
    CUSTOM_YELLOW,
    CUSTOM_BLACK,
    CUSTOM_LIGHT_GREEN,
    CUSTOM_LIGHT_YELLOW,
    CUSTOM_PINK
};


// New PAIRS
enum ColorPairs {
    BG_LOGIN = 0,
    MESSAGE_COLOR,
    BTN_DEFAULT,
    BTN_SELECTED,
    HEADER_COLOR,
    LABEL_COLOR,
    TEXT_COLOR,
    BTN_DEFAULT_2,
    BTN_SELECTED_2,
    
    WHITE_TEXT,
    GREEN_TEXT,
    GREEN_TEXT_WHITE,
    GREEN_TEXT_YELLOW,
    CYAN_TEXT,
    LIGHT_ORANGE_TEXT,
    ORANGE_TEXT,
    YELLOW_TEXT,
    PURPLE_TEXT,
    RED_TEXT,
    BLACK_TEXT,
    LIGHT_GREEN_TEXT,
    LIGHT_YELLOW_TEXT,
    PINK_TEXT
};

// Structs
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
    
    int color_id;
    int last_game_id;

} User;

typedef struct 
{
    int type; // 0: health, 1:speed, 2:damage
    int HP_regen;
    int speed;
    int power; // power: 2 means two times damage
    
    int time; //10 at first
    Loc loc;
} Enchant;

typedef struct 
{
    int type; //type: 0: common, 1: magic, 2: perfect, 3: Bad
    int HP;
    int speed;
    int power;

    int time; //20: magic, 15: perfect, 10: common, 0: bad
    Loc loc;
} Food;

typedef struct 
{
    Loc loc;
    bool visible;
} Trap;

typedef struct 
{
    int type; //type: 0: mace, 1: dagger, 2: magic wood, 3: normal arrow, 4: sword
    int num;
    int damage;
    Loc loc;
} Weapon;

typedef struct 
{
    int type; //type: 0: common, 1: black
    int num;
    Loc loc;
} Gold;

typedef struct 
{
    int type; //type: 0: common, 1: hidden
    bool is_visible;
    Loc loc;
} Door;



typedef struct 
{
    Loc loc;
    int level_num;
    bool is_down; // TRUE: down, FALSE: Up
} Stair;

typedef struct {
    Loc loc;
    int type; // 0: Demon, 1: Fire breathing monster, 2: Giant, 3: Snake, 4: Undeed
    int hp;
    int damage;
    int possible_moves;
} Enemy;

typedef struct 
{
    int type; //type: 0: common, 1: enchant, 2: treasure
    Loc e1, e2, e3, e4;

    int w;
    int h;
    
    Loc window;
    Loc O;

    Door doors[MAX_DOORS_ROOM];
    Trap traps[MAX_OBJ_ROOM];
    Food foods[MAX_OBJ_ROOM];
    Enchant* enchants;
    Gold golds[MAX_OBJ_ROOM];
    Weapon weapons[MAX_WEAPON_ROOM];

    bool is_visited;
} Room;


typedef struct 
{
    int number;
    char** map;
    int** visited;
    Room* rooms;
    int rooms_num;

    int stair_room_index;
    Stair stairs[2];

    Enemy enemys[MAX_ENEMY_LEVEL];

    int mode;
} Level;

typedef struct 
{
    int width, height;
    Level* main_levels;
    // Level* visited_levels;
    int levels_num;
    
    bool show_all;
} Map;

typedef struct {
    Loc now_loc;
    int hp;
    int regen;
    int hungriness;
    int level;

    int points;
    int golds;

    Food foods[5];
    Weapon weapons[5];
    Weapon hand_weapon;
    Enchant enchants[5];
} Player;


typedef struct{
    bool map_show_all;
    bool food_menu_open;
    bool enchant_menu_open;
    bool weapon_menu_open;
    bool quit;
    int visible_r;

    char msg[3][50];
    int msg_num;

} UI_state;

typedef struct{
    int hardness;
    Map* map;
    User* user;
    Player player;
} Game;

// graphics
void add_color_rgb(int id, int r, int g, int b);

void clear_part(WINDOW* w, int y1, int x1, int y2, int x2);

void print_title(WINDOW* w, char* title, int y, int x);

void print_messages(WINDOW* w, char ms[][50], int size, int y, int x, char type, int color_id, int step);

void print_buttons(WINDOW* w, char btns[][50], int size, int selected, int y, int x, int color_mode);

void handle_selected_btn(int* selected, int size, int* flag);


// randoms
int rand_in(int a, int b);

void rand_ints(int* arr, int size, int a, int b, int ascending);
// Math Functions
int min(int a, int b);
int max(int a, int b);

// string functions
bool is_upper(char c);
bool is_lower(char c);
bool is_digit(char c);

// Files
char* read_file(char* path);

int write_file(char* path, char* data);

int count_users();

void read_usernames(char** usernames, int number_of_users);


// User
void update_user_int_data(User t_user, char* detail, int value);

void reset_user_data(User* user);

void get_user_detail_by_username(char* target_username, char* data, char* value);

void save_user_data(User user);

void game_initalize();

#endif