#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ncursesw/ncurses.h>
#include "cjson/cJSON.h"
#include <regex.h>
#include <time.h>

#include "basic_loads.h"

#include "game.h"

char top_details[4][50] = {
    "Level","HP", "Golds", "Score"
};

// کمکی
bool is_empty_room(Level level, Loc loc){
    // mvaddch(1,1, level.map[loc.y][loc.x]);
    // refresh();
    // getch();
    if(level.map[loc.y][loc.x] == '.')
        return TRUE;
    return FALSE;
}

Loc find_empty_place_room(Level level, Room room, int padding){
    Loc res;
    int c = (room.e4.x - room.e1.x - 2*padding) * (room.e4.y - room.e1.y - 2*padding);
    while(c > 0){
        res.x = rand_in(room.e1.x+padding, room.e4.x-padding);
        res.y = rand_in(room.e1.y+padding, room.e4.y-padding);
        c--;
        if(is_empty_room(level, res)) return res;
    }
    // res.x = rand_in(room.e1.x+1, room.e4.x-1);
    // res.y = rand_in(room.e1.y+1, room.e4.y-1);
    // return res;
}

void putch_map(Level* level, int y, int x, char ch) {
    level->map[y][x] = ch;
}

void find_first_place(Game* g, Player* p){
    Room* room = g->map->main_levels[0].rooms;
    Loc loc;
    loc.x = room->e1.x + 1, loc.y = room->e1.y + 1;
    while(!is_empty_room(g->map->main_levels[0], loc)){
        loc.x++;
        if(loc.x == room->e4.x){
            loc.x = 1;
            loc.y++;
        }
    }
    p->now_loc.x = loc.x;
    p->now_loc.y = loc.y;
}

void draw_in_map(WINDOW* win, int y, int x, char* str, int color, bool is_bold){
    if(is_bold){
        wattron(win, A_BOLD);
    }
    wattron(win, COLOR_PAIR(color));
    mvwaddstr(win, y, x, str);
    wattroff(win, COLOR_PAIR(color));
    if(is_bold){
        wattroff(win, A_BOLD);
    }
}

void draw_corridor_two_bends(Level* level, int x1, int y1, int x2, int y2, int mode) {
    if(mode == 1){
        int mid_x = (x1 + x2) / 2;

        int start_x = min(x1, mid_x);
        int end_x   = max(x1, mid_x);
        
        for (int x = start_x; x <= end_x; x++) {
            putch_map(level, y1, x, '#');
        }
        
        int start_y = min(y1, y2);
        int end_y   = max(y1, y2);
        for (int y = start_y; y <= end_y; y++) {
            putch_map(level, y, mid_x, '#');
        }
        
        start_x = min(mid_x, x2);
        end_x   = max(mid_x, x2);
        for (int x = start_x; x <= end_x; x++) {
            putch_map(level, y2, x, '#');
        }
    } else {
        int mid_y = (y1 + y2) / 2;

        int start_y = min(y1, mid_y);
        int end_y   = max(y1, mid_y);
        
        for (int y = start_y; y <= end_y; y++) {
            putch_map(level, y, x1, '#');
        }
        
        int start_x = min(x1, x2);
        int end_x   = max(x1, x2);
        for (int x = start_x; x <= end_x; x++) {
            putch_map(level, mid_y, x, '#');
        }
        
        start_y = min(mid_y, y2);
        end_y   = max(mid_y, y2);
        for (int y = start_y; y <= end_y; y++) {
            putch_map(level, y, x2, '#');
        }
    }
}

void set_room(Room *room, int e1_x, int e1_y, int w, int h){
    room->e1.x = e1_x;
    room->e1.y = e1_y;
    room->w = w;
    room->h = h;
    
    room->e2.x = room->e1.x + w;
    room->e2.y = room->e1.y;
    
    room->e3.x = room->e1.x;
    room->e3.y = room->e1.y + h;
    
    room->e4.x = room->e1.x + w;
    room->e4.y = room->e1.y + h;
    room->window.x = room->window.y = 0;
    room->O.x = room->O.y = 0;
    room->is_visited = false;
}

int find_good_id_for_room(Map* map, int level_num){
    Level* level = &map->main_levels[level_num];
    int res;
    if(level_num == 0){
        res = rand_in(1, level->rooms_num - 1);
    } else{
        // بررسی برای اینکه پله بالا و پایین کنار هم نباشن
        res = rand_in(0, level->rooms_num - 1);
        int pre_stair_room_i = map->main_levels[level_num - 1].stair_room_index;
        if(res == pre_stair_room_i){
            res = pre_stair_room_i - 1 < 0 ? 7 : res - 1;
        }
    }
    return res;
}

Room* get_room_by_loc(Level* level, int x, int y){
    for(int i = 0; i < level->rooms_num; i++){
        Room* r = &level->rooms[i];
        if(x > r->e1.x && x < r->e4.x &&
           y > r->e1.y && y < r->e4.y){
            return r;
        }
    }
    return NULL;
}
Door* get_door_by_room(Room* r, int x, int y){
    for(int i = 0; i < MAX_DOORS_ROOM; i++){
        if(r->doors[i].loc.x == x && r->doors[i].loc.y == y)
            return &r->doors[i];
    }
    return NULL;
}
Gold* get_gold_by_room(Room* r, int x, int y){
    for(int i = 0; i < MAX_OBJ_ROOM; i++){
        if(r->golds[i].loc.x == x && r->golds[i].loc.y == y)
            return &r->golds[i];
    }
    return NULL;
}
Food* get_food_by_room(Room* r, int x, int y){
    for(int i = 0; i < MAX_OBJ_ROOM; i++){
        if(r->foods[i].loc.x == x && r->foods[i].loc.y == y)
            return &r->foods[i];
    }
    return NULL;
}
Trap* get_trap_by_room(Room* r, int x, int y){
    for(int i = 0; i < MAX_OBJ_ROOM; i++){
        if(r->traps[i].loc.x == x && r->traps[i].loc.y == y)
            return &r->traps[i];
    }
    return NULL;
}
Enchant* get_enchant_by_room(Room* r, int x, int y){
    for(int i = 0; i < MAX_OBJ_ROOM; i++){
        if(r->enchants[i].loc.x == x && r->enchants[i].loc.y == y)
            return &r->enchants[i];
    }
    return NULL;
}
Weapon* get_weapon_by_room(Room* r, int x, int y){
    for(int i = 0; i < MAX_OBJ_ROOM; i++){
        if(r->weapons[i].loc.x == x && r->weapons[i].loc.y == y)
            return &r->weapons[i];
    }
    return NULL;
}
Enemy* get_enemy_by_loc(Level* level, int x, int y){
    for(int i = 0; i < MAX_ENEMY_LEVEL; i++){
        if(level->enemys[i].loc.x == x && level->enemys[i].loc.y == y)
            return &level->enemys[i];
    }
    return NULL;
}

bool can_go_loc(Game* g, int y, int x){
    int level_num = g->player.level;
    
    switch(g->map->main_levels[level_num].map[y][x]){
        case '_': case '|': case 'O': case ' ':
            return FALSE;
        // enemies
        case 'F': case 'U': case 'G': case 'S': case 'D':
            return FALSE;
    }
    if(y < 0 || x < 0) return FALSE;

    return TRUE;
}


void show_msg(char * ms, int y, int x, int color, bool is_bold){
    clear_part(stdscr, y, 1, y, COLS - 1);
    if(is_bold) attron(A_BOLD);
    
    attron(COLOR_PAIR(color));
    mvprintw(y,x, "%s", ms);
    attroff(COLOR_PAIR(color));
    if(is_bold) attroff(A_BOLD);

    refresh();
}

void clear_msgs(UI_state* state){
    for(int i = 0; i < state->msg_num; i++){
        clear_part(stdscr, i + 2, 1, i + 2, COLS - 1);
        strcpy(state->msg[i], "");
    }
    state->msg_num = 0;
}

void draw_food_window(Game* g, WINDOW* win, int width, int height) {
    // رسم قاب اصلی پنجره
    box(win, 0, 0);
    refresh();
    // نمایش عنوان یا میزان گرسنگی
    print_title(win, "Hunger: ", 2, 3);
    wmove(win, 2, width / 2);

    Player* player = &g->player;

    int color_id = RED_TEXT;
    if (player->hungriness <= 15)
        color_id = GREEN_TEXT;
    else if (player->hungriness <= 50)
        color_id = ORANGE_TEXT;

    wattron(win, COLOR_PAIR(color_id));
    // رسم نوار گرسنگی (هر واحد گرسنگی یک بلوک)
    for (int i = 0; i < player->hungriness; i++) {
        printw("■");
    }
    wattroff(win, COLOR_PAIR(color_id));

    int num_food = player->foods_num; 
    int box_height = 3;              
    int box_width  = width - 4;       
    int box_start_x = 2;             
    int start_row = 5;               

    for (int i = 0; i < num_food; i++) {
        int box_start_y = start_row + i * (box_height + 1); // فاصله‌ی بین باکس‌ها یک ردیف

        // رسم خط بالا (top border)
        mvwhline(win, box_start_y, box_start_x, '-', box_width);
        // رسم خط پایین (bottom border)
        mvwhline(win, box_start_y + box_height - 1, box_start_x, '-', box_width);
        // رسم خطوط کناری (left و right borders)
        mvwvline(win, box_start_y, box_start_x, '|', box_height);
        mvwvline(win, box_start_y, box_start_x + box_width - 1, '|', box_height);
        // رسم گوشه‌ها
        mvwaddch(win, box_start_y, box_start_x, '+');
        mvwaddch(win, box_start_y, box_start_x + box_width - 1, '+');
        mvwaddch(win, box_start_y + box_height - 1, box_start_x, '+');
        mvwaddch(win, box_start_y + box_height - 1, box_start_x + box_width - 1, '+');

        // محاسبه‌ی ردیف میانی باکس (برای قرار دادن آیکون و تعداد)
        int content_y = box_start_y + 1;


        Food* food = &player->foods[i];
        int color;
        if(food->type == 0){
            color = LIGHT_GREEN_TEXT;
        } else if(food->type == 0){
            color = CYAN_TEXT;
        }else if(food->type == 0){
            color = LIGHT_YELLOW_TEXT;
        } else{
            color = RED_TEXT;
        }
        draw_in_map(win, content_y, box_start_x + 2, "⊙", color, FALSE);

        // چاپ تعداد غذا در سمت راست نزدیک به حاشیه
        mvwprintw(win, content_y, box_start_x + box_width - 4, "%d", 1);
    }

    // به‌روزرسانی پنجره جهت نمایش تغییرات
    wrefresh(win);
}

// توابع اصلی
void initialize_map(Game* g){
    Map* map = malloc(sizeof(Map));
    g->map = map;
    
    int levels_num = 4;
    map->levels_num = levels_num;
    map->main_levels = (Level*)malloc(levels_num * sizeof(Level));
    // map->visited_levels = (Level*)malloc(levels_num * sizeof(Level));
    map->show_all = TRUE;

    int width = COLS - 22, height = 30;
    map->width = width;
    map->height = height;
    // For each level
    for(int i = 0; i < levels_num; i++){
        map->main_levels[i].number = i+1;
        map->main_levels[i].visited = (int**)malloc(height * sizeof(int*));
        map->main_levels[i].map = (char**)malloc(height * sizeof(char*));
        for(int j = 0; j < height; j++){
            map->main_levels[i].map[j] = (char*)malloc(width * sizeof(char));
            for(int z = 0; z < width; z++){
                map->main_levels[i].map[j][z] = ' ';
            }
            map->main_levels[i].visited[j] = (int*)calloc(width, sizeof(int));
        }
        map->main_levels[i].rooms_num = 8;
        
        Room* map_rooms = malloc(map->main_levels[i].rooms_num * sizeof(Room));
        map->main_levels[i].rooms = map_rooms;
    }
}

void connect_door_pairs(Level *level) {
    // دوری
    for(int i = 0; i < 3; i++){
        draw_corridor_two_bends(level, level->rooms[i].doors[1].loc.x,
                                    level->rooms[i].doors[1].loc.y,
                                    level->rooms[i+1].doors[0].loc.x,
                                    level->rooms[i+1].doors[0].loc.y, 1);
    }
    for(int i = 4; i < 7; i++){
        draw_corridor_two_bends(level, level->rooms[i].doors[1].loc.x,
                                    level->rooms[i].doors[1].loc.y,
                                    level->rooms[i+1].doors[0].loc.x,
                                    level->rooms[i+1].doors[0].loc.y, 1);
    }
    draw_corridor_two_bends(level, level->rooms[0].doors[0].loc.x,
                                    level->rooms[0].doors[0].loc.y,
                                    level->rooms[4].doors[0].loc.x,
                                    level->rooms[4].doors[0].loc.y, 2);
    
    draw_corridor_two_bends(level, level->rooms[7].doors[1].loc.x,
                                level->rooms[7].doors[1].loc.y,
                                level->rooms[3].doors[1].loc.x,
                                level->rooms[3].doors[1].loc.y, 2);

    // فرعی ها
    draw_corridor_two_bends(level, level->rooms[1].doors[2].loc.x,
                                    level->rooms[1].doors[2].loc.y,
                                    level->rooms[5].doors[2].loc.x,
                                    level->rooms[5].doors[2].loc.y, 2);
    draw_corridor_two_bends(level, level->rooms[2].doors[2].loc.x,
                                    level->rooms[2].doors[2].loc.y,
                                    level->rooms[6].doors[2].loc.x,
                                    level->rooms[6].doors[2].loc.y, 2);                                
}

void add_doors_to_room(int i, Room *room, int mode) {
    // اصلی ها
    switch(i){
        case 0:
            room->doors[1].loc.x = room->e4.x;
            room->doors[1].loc.y = rand_in(room->e1.y + 2 ,room->e4.y -2);

            room->doors[0].loc.x = rand_in(room->e1.x + 2 ,room->e4.x -2);
            room->doors[0].loc.y = room->e4.y;
            break;
        case 3:
            room->doors[0].loc.x = room->e1.x;
            room->doors[0].loc.y = rand_in(room->e1.y + 2 ,room->e4.y -2);

            room->doors[1].loc.x = rand_in(room->e1.x + 3 ,room->e4.x -3);
            room->doors[1].loc.y = room->e4.y;
            break;
        case 4:
            room->doors[1].loc.x = room->e4.x;
            room->doors[1].loc.y = rand_in(room->e1.y + 2 ,room->e4.y -2);

            room->doors[0].loc.x = rand_in(room->e1.x + 2 ,room->e4.x -2);
            room->doors[0].loc.y = room->e1.y;
            break;
        case 7:
            room->doors[1].loc.x = rand_in(room->e1.x + 2 ,room->e4.x -2);
            room->doors[1].loc.y = room->e1.y;

            room->doors[0].loc.x = room->e1.x;
            room->doors[0].loc.y = rand_in(room->e1.y + 2 ,room->e4.y -2);
            break;
        default:
            room->doors[0].loc.x = room->e1.x;
            room->doors[0].loc.y = rand_in(room->e1.y + 2 ,room->e4.y -2);

            room->doors[1].loc.x = room->e2.x;
            room->doors[1].loc.y = rand_in(room->e1.y + 2 ,room->e4.y -2);
    }

    // فرعی ها
    switch(i){
        case 1: case 2:
            room->doors[2].loc.x = rand_in(room->e1.x + 2 ,room->e4.x - 2);
            room->doors[2].loc.y = room->e4.y;
            break;
        case 5: case 6:
            room->doors[2].loc.x = rand_in(room->e1.x + 2 ,room->e4.x - 2);
            room->doors[2].loc.y = room->e1.y;
            
            break;
    }
    // anchents
    switch(mode){
        case 2: // room 2
            if(i == 1)
                room->doors[1].type = 1;
            else if(i == 3)
                room->doors[0].type = 1;
            else if(i == 6)
                room->doors[2].type = 1;
            else if(i == 2)
                room->type = 1;
                
            break;
        case 3: //room 5 
            if(i == 4)
                room->doors[1].type = 1;
            else if(i == 6)
                room->doors[0].type = 1;
            else if(i == 1)
                room->doors[2].type = 1;
            else if(i == 6)
                room->type = 1;
            break;
        case 4: //room 6 
            if(i == 5)
                room->doors[1].type = 1;
            else if(i == 7)
                room->doors[0].type = 1;
            else if(i == 2)
                room->doors[2].type = 1;
            else if(i == 6)
                room->type = 1;
            break;
        case 5: //room 7 
            if(i == 3 || i == 6)
                room->doors[1].type = 1;
            else if(i == 7)
                room->type = 1;
            break;
         
    }
}

void make_rooms_and_corridors(Game* g, int h, int w, Map* map, int level_num){
    
    Level* level = &map->main_levels[level_num];
    if(level_num == 0) level->rooms[0].is_visited = 1;
    else{
        Level* pre_level = &map->main_levels[level_num - 1];
        int id = pre_level->stair_room_index;
        level->rooms[id].is_visited = 1;
    }
    level->mode = rand_in(2,5); //mode 1: none hidden / mode 2: 2 anchent / mode 3: 5 anchent
                                // mode 4: 6 / mode 5: 7
    switch(level->mode){
        case 2:
            level->rooms[2].type = 1;
            break;
        case 3:
            level->rooms[5].type = 1;
            break;
        case 4:
            level->rooms[6].type = 1;
            break;
        case 5:
            level->rooms[7].type = 1;
            break;
    }

    // for each room
    for (int i = 0; i < level->rooms_num; i++){
        Room *r = &level->rooms[i];
        int color_id = WHITE_TEXT;
        
        
        putch_map(level, r->e1.y, r->e1.x, '.');
        putch_map(level, r->e2.y, r->e2.x, '.');
        putch_map(level, r->e3.y, r->e3.x, '|');
        putch_map(level, r->e4.y, r->e4.x, '|');

        for (int j = r->e1.x + 1; j < r->e2.x; j++){
            putch_map(level, r->e1.y, j, '_');
            putch_map(level, r->e4.y, j, '_');
        }
        for (int j = r->e1.y + 1; j < r->e3.y; j++){
            putch_map(level, j, r->e1.x, '|');
            putch_map(level, j, r->e4.x, '|');
        }
        for (int j = r->e1.y + 1; j < r->e3.y; j++){
            for (int z = r->e1.x + 1; z < r->e2.x; z++){
                putch_map(level, j, z, '.');
            }
        }
        
        // //window
        // r->window.x = rand_in(r->e1.x + 1, r->e2.x - 1);
        // r->window.y = (rand() % 2 == 0) ? r->e1.y : r->e4.y;
        // putch_map(level, r->window.y, r->window.x, '=');

        add_doors_to_room(i,r, level->mode);
    }
    connect_door_pairs(level);

    // for each room
    for (int i = 0; i < level->rooms_num; i++){
        Room *r = &level->rooms[i];
        // adding doors
        int color_id = WHITE_TEXT;
        if(r->type == 1) color_id = PURPLE_TEXT;
        
        for (int d = 0; d < 4; d++){
            if (r->doors[d].loc.x > 0 && r->doors[d].loc.y > 0){
                if(r->doors[d].type == 1){
                    putch_map(level, r->doors[d].loc.y, r->doors[d].loc.x, '?');
                } else{
                    putch_map(level, r->doors[d].loc.y, r->doors[d].loc.x, '+');
                }
            }
        }

        //O
        r->O.x = rand_in(r->e1.x + 2, r->e2.x - 2);
        r->O.y = rand_in(r->e1.y + 2, r->e4.y - 2);
        putch_map(level, r->O.y, r->O.x, 'O');
        // trap (hidden)
        int traps_num;
        if(r->type == 0){
            traps_num = rand_in(1 + r->w/5 + r->h/5, 3 + r->w/5 + r->h/5);
        } else if (r -> type == 1){
            traps_num = rand_in(0 + r->w/5 + r->h/5, 1 + r->w/5 + r->h/5);
        } else if(r -> type == 2){
            traps_num = rand_in(4, 6 + r->w/5 + r->h/5);
        }
        
        for(int j = 0; j < traps_num; j ++){
            r->traps[j].loc = find_empty_place_room(*level, *r, 2);
            r->traps->visible = FALSE;
            putch_map(level, r->traps[j].loc.y, r->traps[j].loc.x, 'X');
        }
        // golds
        int golds_num, rand_num;
        if(r->type == 0){
            golds_num = rand_in(0, 5 + r->w/5 + r->h/5);
        } else if (r -> type == 1){
            golds_num = rand_in(0, 3 + r->w/5 + r->h/5);
        } else if(r -> type == 2){
            golds_num = rand_in(2, 3 + r->w/5 + r->h/5);
        }
        for(int j = 0; j < golds_num; j ++){
            r->golds[j].loc = find_empty_place_room(*level, *r, 1);
            rand_num = rand_in(1, 10);
            if(rand_num <= 9){
                r->golds[j].type = 0;
                r->golds[j].num = 2;
            } else{
                r->golds[j].type = 1;
                r->golds[j].num = 10;
            }
            putch_map(level, r->golds[j].loc.y, r->golds[j].loc.x, '$');
        } 

        // foods
        int foods_num;
        if(r->type == 0){
            foods_num = rand_in(0 + r->w/7 + r->h/10, 1 + r->w/7 + r->h/10);
        } else if (r -> type == 1){
            foods_num = rand_in(0 + r->w/7 + r->h/10, 1 + r->w/7 + r->h/10);
        } else if(r -> type == 2){
            foods_num = 1;
        }
        for(int j = 0; j < foods_num; j ++){
            r->foods[j].loc = find_empty_place_room(*level, *r, 1);
            rand_num = rand_in(1, 100);
            if(rand_num <= 50){ // common
                r->foods[j].type = 0;
                r->foods[j].HP = 100;
                r->foods[j].time = 10;
            } else if (rand_num <= 65){ //magic
                r->foods[j].type = 1;
                r->foods[j].HP = 50;
                r->foods[j].speed = 20;
                r->foods[j].time = 20;
            }else if (rand_num <= 80){ // perfect
                r->foods[j].type = 2;
                r->foods[j].HP = 50;
                r->foods[j].power = 20;
                r->foods[j].time = 15;
            } else { // Bad
                r->foods[j].type = 3;
                r->foods[j].HP = -50;
                r->foods[j].time = 0;
            }
            putch_map(level, r->foods[j].loc.y, r->foods[j].loc.x, 'N');
        } 

        // enchants
        if(r->type == 1){
            int enchant_num = rand_in(2 + r->w/5 + r->h/5, 5 + r->w/5 + r->h/5);
            r->enchants = (Enchant*)malloc(enchant_num * sizeof(Enchant));
            for(int j = 0; j < enchant_num; j ++){
                r->enchants[j].loc = find_empty_place_room(*level, *r, 1);
                rand_num = rand_in(0, 2);
                if (rand_num == 0){
                    r->enchants[j].type = 0;
                    r->enchants[j].HP_regen = 12;
                } else if (rand_num == 1){
                    r->enchants[j].type = 1;
                    r->enchants[j].speed = 2;
                } else if (rand_num == 2){
                    r->enchants[j].type = 2;
                    r->enchants[j].power = 2;
                }
                r->enchants[j].time = 10;
                putch_map(level, r->enchants[j].loc.y, r->enchants[j].loc.x, 'E');
            }

        }

        // weapons
        int weapons_num = rand_in(0, 1 + r->w/7 + r->h/7);
        
        for(int j = 0; j < weapons_num; j ++){
            r->weapons[j].loc = find_empty_place_room(*level, *r, 1);
            rand_num = rand_in(1, 4);
            if (rand_num == 1){
                r->weapons[j].type = 1;
                r->weapons[j].num = 10;
                r->weapons[j].damage = 12;
            }else if (rand_num == 2){
                r->weapons[j].type = 2;
                r->weapons[j].num = 8;
                r->weapons[j].damage = 15;
            } else if (rand_num == 3){
                r->weapons[j].type = 3;
                r->weapons[j].num = 5;
                r->weapons[j].damage = 5;
            } else {
                r->weapons[j].type = 4;
                r->weapons[j].num = 1;
                r->weapons[j].damage = 10;
            }
            putch_map(level, r->weapons[j].loc.y, r->weapons[j].loc.x, 'W');
        }
        // enenmies
        int enemy_num = rand_in(1, 1 + r->w/7 + r->h/7);
        if(i == 0) enemy_num = rand_in(0,1);
        if(r->type == 2){
            enemy_num = rand_in(4, 4 + r->w/7 + r->h/7);
        }
        for(int j = 0; j < enemy_num; j ++){
            level->enemys[j].loc = find_empty_place_room(*level, *r, 1);

            rand_num = rand_in(0, 4);
            if (rand_num == 0){
                level->enemys[j].type = 0;
                level->enemys[j].damage = 5;
                level->enemys[j].hp = 5;
                level->enemys[j].possible_moves = 0;
                putch_map(level, r->traps[j].loc.y, r->traps[j].loc.x, 'D');
            }else if (rand_num == 1){
                level->enemys[j].type = 1;
                level->enemys[j].damage = 5;
                level->enemys[j].hp = 10;
                level->enemys[j].possible_moves = 0;
                putch_map(level, r->traps[j].loc.y, r->traps[j].loc.x, 'F');
            } else if (rand_num == 2){
                level->enemys[j].type = 2;
                level->enemys[j].damage = 5;
                level->enemys[j].hp = 15;
                level->enemys[j].possible_moves = 5;
                putch_map(level, r->traps[j].loc.y, r->traps[j].loc.x, 'G');
            }else if (rand_num == 3){
                level->enemys[j].type = 3;
                level->enemys[j].damage = 5;
                level->enemys[j].hp = 20;
                level->enemys[j].possible_moves = 1000;
                putch_map(level, r->traps[j].loc.y, r->traps[j].loc.x, 'S');
            } else {
                level->enemys[j].type = 4;
                level->enemys[j].damage = 5;
                level->enemys[j].hp = 30;
                level->enemys[j].possible_moves = 5;
                putch_map(level, r->traps[j].loc.y, r->traps[j].loc.x, 'U');
            }
            
        }
    }

    // Stair
    // // To down
    int stair_room_index, color_id;
    stair_room_index = find_good_id_for_room(map, level_num);
    if(level_num == 3){
        level->rooms[stair_room_index].type = 2;
        color_id = GREEN_TEXT_YELLOW;
    } else{
        color_id = GREEN_TEXT_WHITE;
    }
    level->stair_room_index = stair_room_index;
    Room* t_room = &level->rooms[stair_room_index];
    
    Stair* stair = &level->stairs[0];
    stair->level_num = level->number;
    stair->loc = find_empty_place_room(*level, *t_room,1);
    stair->is_down = TRUE;
    putch_map(level, stair->loc.y, stair->loc.x, '>');


    // // To up
    if(level_num != 0){
        Stair* stair_up = &level->stairs[1];
        stair_up->is_down = FALSE;
        stair_up->level_num = level_num;
        Room* last_room = map->main_levels[level_num - 1].rooms;
        stair_up->loc = map->main_levels[level_num - 1].stairs[0].loc;
        
        putch_map(level, stair_up->loc.y, stair_up->loc.x, '<');
    }
}

void make_random_map(Game* g){
    int width = g->map->width, height = g->map->height;
    // setting rooms & Levels
    
    // For each level
    for(int j = 0; j < g->map->levels_num; j++){
        int rooms_num = g->map->main_levels[j].rooms_num;
        Room* map_rooms = g->map->main_levels[j].rooms;
        for (int i = 0; i < rooms_num; i++){
            int x_s = (width / 4) * (i % 4) + 1;
            int x_f = (width / 4) * ((i % 4) + 1) - 1;
            int y_s = (height / 2) * (i / 4) + 1;
            int y_f = (height / 2) * ((i / 4) + 1) - 1;
            
            int w_room = rand_in(10, 13);
            int h_room = rand_in(7, 10);
            
            set_room(&map_rooms[i], rand_in(x_s, x_f - w_room - 1), rand_in(y_s, y_f - h_room - 1), w_room, h_room);
        }
        if(j != 0){
            Level pre_lev = g->map->main_levels[j-1];
            int i = pre_lev.stair_room_index;
            Room* last_level_rooms = pre_lev.rooms;
            set_room(&map_rooms[i], last_level_rooms[i].e1.x, last_level_rooms[i].e1.y,
                     last_level_rooms[i].w, last_level_rooms[i].h);
            
        }
        
        make_rooms_and_corridors(g, height, width, g->map, j);
    }
}


void draw_game_map(Game *g, WINDOW* win, int level_num, UI_state state){
    Level* level = &g->map->main_levels[level_num];
    char** map = level->map;
    int height = g->map->height, width = g->map->width;
    // # s
    for(int j = 1; j < height - 1; j++){
        for(int i = 1; i < width - 1; i++){
            char letter[5];
            int color_id;

            if(map[j][i] == ' ' || !level->visited[j][i]) draw_in_map(win, j, i, " ", WHITE_TEXT, FALSE);
            if(map[j][i] == '#' && (level->visited[j][i] == 1 || state.map_show_all)) draw_in_map(win, j, i, "#", ORANGE_TEXT, FALSE);
        }
    }

    for(int z = 0; z < level->rooms_num; z++){
        Room* r = &level->rooms[z];
        if(r->is_visited == FALSE && !state.map_show_all) continue;
        int color_id = WHITE_TEXT;
        if(r->type == 1) color_id = PURPLE_TEXT;
        if(r->type == 2) color_id = YELLOW_TEXT;
        for(int j = r->e1.y; j <= r->e4.y; j++){
            for(int i = r->e1.x; i <= r->e4.x; i++){
                char ch[2];
                ch[0] = map[j][i];
                ch[1] = '\0';
                switch(*ch){
                    case 'X':
                        Trap* trap = get_trap_by_room(r, i, j);
                        if(state.map_show_all || trap->visible) draw_in_map(win, j, i, ch, RED_TEXT, TRUE);
                        else draw_in_map(win, j, i, ".", color_id, FALSE);
                        break;
                    case '$':
                        Gold* gold = get_gold_by_room(r, i, j);
                        if(gold->num == 0){
                            draw_in_map(win, j, i, ".", color_id, FALSE);
                            break;
                        }
                        if(gold->type == 0){
                            draw_in_map(win, j, i, "⛂", YELLOW_TEXT, FALSE);
                        }
                        else if(gold->type == 1){
                            draw_in_map(win, j, i, "⛂", BLACK_TEXT, FALSE);
                        }
                        break;
                    case 'N':
                        Food* food = get_food_by_room(r, i, j);
                        int color;
                        if(food->type == 0){
                            color = LIGHT_GREEN_TEXT;
                        } else if(food->type == 0){
                            color = CYAN_TEXT;
                        }else if(food->type == 0){
                            color = LIGHT_YELLOW_TEXT;
                        } else{
                            color = RED_TEXT;
                        }
                        draw_in_map(win, j, i, "⊙", color, FALSE);
                        break;
                    case 'W':
                        Weapon* weapon = get_weapon_by_room(r, i, j);
                        
                        if(weapon->type == 0){
                            draw_in_map(win, j, i, "𓌉", WHITE_TEXT, FALSE);
                        }else if(weapon->type == 1){
                            draw_in_map(win, j, i, "⚔", WHITE_TEXT, FALSE);
                        }else if(weapon->type == 2){
                            draw_in_map(win, j, i, "⚚", WHITE_TEXT, FALSE);
                        }else if(weapon->type == 3){
                            draw_in_map(win, j, i, "➳", WHITE_TEXT, FALSE);
                        }else if(weapon->type == 4){
                            draw_in_map(win, j, i, "🗡", WHITE_TEXT, FALSE);
                        }
                        break;

                    case 'D': case 'F': case 'G': case 'S': case 'U':
                        draw_in_map(win, j, i, ch, LIGHT_YELLOW_TEXT, TRUE);
                        break;
                    case 'E':
                        Enchant* enchant = get_enchant_by_room(r, i, j);
                        
                        if(enchant->type == 0){
                            draw_in_map(win, j, i, "⚕", PINK_TEXT, FALSE);
                        }else if(enchant->type == 1){
                            draw_in_map(win, j, i, "⛷", PINK_TEXT, FALSE);
                        }else if(enchant->type == 2){
                            draw_in_map(win, j, i, "Ψ", PINK_TEXT, FALSE);
                        }
                        break;
                    case '>':
                        if(level->number == 4){
                            draw_in_map(win, j, i, ch, GREEN_TEXT_YELLOW, TRUE);
                        } else{
                            draw_in_map(win, j, i, ch, GREEN_TEXT_WHITE, TRUE);
                        }
                        break;
                    case '<':
                        draw_in_map(win, j, i, ch, GREEN_TEXT_WHITE, TRUE);
                        break;
                    case '?':
                        Door* door_hidden = get_door_by_room(r, i, j);
                        if(door_hidden->is_visible || state.map_show_all) draw_in_map(win, j, i, ch, color_id, FALSE);
                        else{
                            if(door_hidden->loc.x == r->e1.x || door_hidden->loc.x == r->e4.x)
                                draw_in_map(win, j, i, "|", color_id, FALSE);
                            else
                                draw_in_map(win, j, i, "_", color_id, FALSE);
                        }
                        break;
                    default:
                        draw_in_map(win, j, i, ch, color_id, FALSE);
                }
            }
        }
    }
    wrefresh(win);
}

void show_visible_corridor(Game *g, WINDOW* win, int level_num, int visible_r, UI_state* state){
    int x = g->player.now_loc.x;
    int y = g->player.now_loc.y;
    char** map_char = g->map->main_levels[level_num].map;
    for(int i = -1* visible_r; i <= visible_r; i++){
        if(get_room_by_loc(&g->map->main_levels[level_num], x, y)) break;
        for(int j = -1* visible_r; j <= visible_r; j++){
            if(x+i < 0 || y+j < 0) continue;
            if(i == 0 && j == 0) continue;
            switch(map_char[y + j][x + i]){
                case '#':
                    draw_in_map(win, y + j, x + i, "#", ORANGE_TEXT, FALSE);
                    break;
                case 'F': case 'U': case 'G': case 'S': case 'D':
                    char str[2];
                    str[0] = map_char[y + j][x + i];
                    str[1] = '\0';
                    draw_in_map(win, y + j, x + i, str, LIGHT_YELLOW_TEXT, TRUE);
                    break;
            }
        }
    }
    for(int i = -1; i <= 1; i++){
        for(int j = -1; j <= 1; j++){  
            Room* r = get_room_by_loc(&g->map->main_levels[level_num], x, y);

            if(x+i < 0 || y+j < 0) continue;
            if(i == 0 && j == 0) continue;
            switch(map_char[y + j][x + i]){
                case '?':
                    clear_msgs(state);
                    state->msg_num = 1;
                    strcpy(state->msg[0], "You found a hidden door!");
                    show_msg(state->msg[0], 2, 2, LIGHT_YELLOW_TEXT, TRUE);

                    draw_in_map(win, y + j, x + i, "?", ORANGE_TEXT, FALSE);
                    if(r) get_door_by_room(r, x + i ,y + j)->is_visible = TRUE;
                    break;
                case 'X':
                    clear_msgs(state);
                    state->msg_num = 1;
                    strcpy(state->msg[0], "Be aware of Trap (X)!");
                    show_msg(state->msg[0], 2, 2, RED_TEXT, TRUE);

                    draw_in_map(win, y + j, x + i, "X", RED_TEXT, TRUE);
                    if(r) get_trap_by_room(r, x + i ,y + j)->visible = TRUE;
                    break;
            }
        }
    }
    wrefresh(win);
}

void draw_player(Game *g, WINDOW* win){
    int x = g->player.now_loc.x;
    int y = g->player.now_loc.y;
    
    g->map->main_levels[g->player.level].visited[y][x] = 1;
    
    int color;
    if(g->user->color_id) color = g->user->color_id;
    else color = WHITE_TEXT;

    wattron(win, A_BOLD | COLOR_PAIR(color));
    mvwprintw(win, y, x, "@");
    wrefresh(win);
    wattroff(win, A_BOLD | COLOR_PAIR(color));
    refresh();
}

void load_player_detail(Game* g, UI_state* state){
    // Top of game page details
    clear_part(stdscr, 1, 1, 1 , COLS-1);
    
    Player* player = &g->player;
    
    // Level
    attron(COLOR_PAIR(LABEL_COLOR) | A_BOLD);
    mvprintw(1,COLS/2 - 10 - strlen(top_details[0])/2, "%s: ", top_details[0]);
    attroff(COLOR_PAIR(LABEL_COLOR) | A_BOLD);
    printw("%d", g->player.level + 1);
    

    // Health
    attron(COLOR_PAIR(LABEL_COLOR) | A_BOLD);
    mvprintw(1,COLS/2 + 10 - strlen(top_details[1])/2, "%s: ", top_details[1]);
    attroff(COLOR_PAIR(LABEL_COLOR)| A_BOLD);

    int color_health = LIGHT_GREEN_TEXT;
    if(g->player.hp <= 0) state->ended_game = TRUE;
    else if(g->player.hp <= 15) color_health = RED_TEXT;
    else if(g->player.hp <= 50) color_health = LIGHT_ORANGE_TEXT;

    attron(COLOR_PAIR(color_health));
    for(int i = 0; i < (g->player.hp)/10; i++){
        printw("■");
    }
    attroff(COLOR_PAIR(color_health));

    // Golds
    attron(COLOR_PAIR(YELLOW_TEXT) | A_BOLD);
    mvprintw(1,COLS/2 + 30, "%s: ", top_details[2]);
    attroff(COLOR_PAIR(YELLOW_TEXT)| A_BOLD);
    printw("%d", g->player.golds);

    // score
    attron(COLOR_PAIR(LABEL_COLOR) | A_BOLD);
    mvprintw(1,COLS/2 + 45, "%s: ", top_details[3]);
    attroff(COLOR_PAIR(LABEL_COLOR)| A_BOLD);
    printw("%d", g->player.points);

    refresh();

    // Set room visible
    int x = g->player.now_loc.x, y = g->player.now_loc.y;
    
    Room* here_room;
    bool room_found = FALSE;
    for(int add_x = -2; add_x <= 2; add_x++){
        for(int add_y = -2; add_y <= 2; add_y++){
            if(room_found) continue;
            int new_x = x + add_x;
            int new_y = y + add_y;
            
            if (new_x >= 0 && new_y >= 0) {
                here_room = get_room_by_loc(&g->map->main_levels[g->player.level], new_x, new_y);
                if (here_room && !here_room->is_visited) {
                    here_room->is_visited = TRUE;
                    room_found = TRUE;
                }
            }
        }
    }
    
    // Enchent Room
    if(here_room && here_room -> type == 1){
        int damage;
        if(g->hardness <= 3){
            damage = 1;
        } else if(g->hardness <= 7){
            damage = 3;
        } else if(g->hardness <= 10){
            damage = 4;
        }
        g->player.hp -= damage;
    }

}

void get_food_here(Game *g, Level* level, int x, int y, UI_state* state){
    if(g->player.foods_num < 5){
        Room* r = get_room_by_loc(level, x, y);

        Food* f = get_food_by_room(r, x, y);
        f->loc.x = 0;
        f->loc.y = 0;

        level->map[y][x] = '.';
        
        g->player.foods[g->player.foods_num++] = *f; 
    } else{
        clear_msgs(state);
        state->msg_num = 2;
        strcpy(state->msg[0], "Your pack is full of food!");
        strcpy(state->msg[1], "Eat one of foods!");
        show_msg(state->msg[0], 2, 2, WHITE_TEXT, TRUE);
        show_msg(state->msg[1], 3, 2, WHITE_TEXT, TRUE);
    }
}

void handle_key(Game* g, UI_state* state, WINDOW* pack_box){
    keypad(stdscr, TRUE);
    int ch = getch();
    int x = g->player.now_loc.x, y = g->player.now_loc.y;
    bool move = FALSE;

    Level* level = &g->map->main_levels[g->player.level];
    char** map = g->map->main_levels[g->player.level].map;
    switch(ch){
        // Movement
        case KEY_UP:case '8':
            y = g->player.now_loc.y - 1;
            move = TRUE;
            break;
        case KEY_DOWN:case '2':
            y = g->player.now_loc.y + 1;
            move = TRUE;
            break;
        case KEY_RIGHT:case '6':
            x = g->player.now_loc.x + 1;
            move = TRUE;
            break;
        case KEY_LEFT:case '4':
            x = g->player.now_loc.x - 1;
            move = TRUE;
            break;
        case '9':
            x = g->player.now_loc.x + 1;
            y = g->player.now_loc.y - 1;
            move = TRUE;
            break;
        case '3':
            x = g->player.now_loc.x + 1;
            y = g->player.now_loc.y + 1;
            move = TRUE;
            break;
        case '1':
            x = g->player.now_loc.x - 1;
            y = g->player.now_loc.y + 1;
            move = TRUE;
            break;
        case '7':
            x = g->player.now_loc.x - 1;
            y = g->player.now_loc.y - 1;
            move = TRUE;
            break;
        
        // Show map
        case 'm': case 'M':
            state->map_show_all = (state->map_show_all) ? FALSE : TRUE;
            break;

        // stairs
        case '>':
            if(map[y][x] == '>'){
                if(g->player.level == 3) state->quit = TRUE;
                else g->player.level++;
            }
            break;
        case '<':
            if(map[y][x] == '<'){
                g->player.level--;
            }
            break;
        // Getting Things 
        case 'g': case 'G':
            // Food
            if(map[y][x] == 'N'){
                get_food_here(g, level, x, y, state);
                state->food_menu_open = TRUE;
            } else{
                state->food_menu_open = state->food_menu_open == TRUE ? FALSE : TRUE;
            }
            break;


        case 'q': case 'Q':
            state->quit = TRUE;
            break;
        case KEY_ESC:
            state->ended_game = TRUE;
    }

    if(move && can_go_loc(g, y, x)){
        g->player.now_loc.x = x;
        g->player.now_loc.y = y;
    }
}

void get_object(Game* g, int x, int y, UI_state* state){
    char** map = g->map->main_levels[g->player.level].map;
    Room* r = get_room_by_loc(&g->map->main_levels[g->player.level], x, y);
    Player* player = &g->player;
    switch(map[y][x]){
        case '$':
            Gold* gold = get_gold_by_room(r, x, y);
            g->player.golds += gold->num;
            g->player.points += gold->num * 10;
            gold->num = 0;
            break;
        case '>':
            clear_msgs(state);
            state->msg_num = 2;
            strcpy(state->msg[0], "You found a Stair to DOWN!");
            strcpy(state->msg[1], "Press > To go down");
            show_msg(state->msg[0], 2, 2, WHITE_TEXT, TRUE);
            show_msg(state->msg[1], 3, 2, WHITE_TEXT, TRUE);
            break;
        case '<':
            clear_msgs(state);
            state->msg_num = 2;
            strcpy(state->msg[0], "Stair to UP!");
            strcpy(state->msg[1], "Press < To go UP!");
            show_msg(state->msg[0], 2, 2, WHITE_TEXT, TRUE);
            show_msg(state->msg[1], 3, 2, WHITE_TEXT, TRUE);
            break;
        case 'X':
            player->hp -= 2 + g->hardness;

            clear_msgs(state);
            state->msg_num = 1;
            strcpy(state->msg[0], "Trap injured you!");
            show_msg(state->msg[0], 2, 2, RED_TEXT, TRUE);
        // Food
        case 'N':
            clear_msgs(state);
            state->msg_num = 2;
            strcpy(state->msg[0], "You found a Food!");
            strcpy(state->msg[1], "Press G To get");
            show_msg(state->msg[0], 2, 2, WHITE_TEXT, TRUE);
            show_msg(state->msg[1], 3, 2, WHITE_TEXT, TRUE);
            break;
        // case 'W':
        //     break;
    }
}

void free_game(Game* g) {
    if (g == NULL || g->map == NULL)
        return;

    Map* map = g->map;

    
    for (int i = 0; i < map->levels_num; i++) {
        Level* level = &map->main_levels[i];

        if (level->map != NULL) {
            for (int j = 0; j < map->height; j++) {
                free(level->map[j]);
            }
            free(level->map);
        }

        if (level->visited != NULL) {
            for (int j = 0; j < map->height; j++) {
                free(level->visited[j]);
            }
            free(level->visited);
        }

        if (level->rooms != NULL) {
            free(level->rooms);
        }
    }
    free(map->main_levels);

    free(map);
    g->map = NULL;
}


int load_main_game(Game* g){
    clear();
    start_color();
    game_initalize();

    Player* player = (Player *)malloc(sizeof(Player));
    player->level = 0;
    player->hp = 100;
    player->regen = 5;
    player->hungriness = 0;
    player->points = 0;
    player->golds = 0;
    player->foods_num = 0;
    

    g->player = *player;
    initialize_map(g);
    make_random_map(g);
    
    int width = g->map->width, height = g->map->height;
    int x_w = 1;
    int y_w = LINES/2 - height/2;

    WINDOW* main_game = newwin(height, width, y_w, x_w);

    int startx = COLS - 22;       
    int starty = (LINES / 2) - 15;  
    int pack_width  = 20;              
    int pack_height = 30;              

    WINDOW *pack_box = newwin(pack_height, pack_width, starty, startx);

    UI_state state;
    state.enchant_menu_open = state.food_menu_open = state.weapon_menu_open = state.map_show_all = state.quit = state.ended_game = FALSE;
    state.visible_r = 2;

    draw_game_map(g, main_game, player->level, state);
    find_first_place(g, &g->player);
    draw_player(g, main_game);

    refresh();
    box(main_game, 0, 0);
    wrefresh(main_game);
    load_player_detail(g, &state);
    
    state.msg_num = 3;
    strcpy(state.msg[0], "Welcome!");
    strcpy(state.msg[1], "Use Arrow key in Num Pad to move. Q: Quit & Save");
    strcpy(state.msg[2], "E: Foods List / i: Weapons List / C: Enchants List");
    for(int i = 0; i < state.msg_num; i++){
        show_msg(state.msg[i], i + 2, 2, WHITE_TEXT, TRUE);
    }
    
    Loc last_place;
    while(1){
        last_place = g->player.now_loc;

        handle_key(g, &state, pack_box);
        clear_msgs(&state);
        if(state.food_menu_open){
            draw_food_window(g, pack_box, pack_width, pack_height);
        }else{
            clear_part(stdscr, starty, startx + 1, starty + pack_height, startx + pack_width);
        }
        draw_game_map(g, main_game, g->player.level, state);
        draw_player(g, main_game);
        show_visible_corridor(g, main_game, g->player.level, state.visible_r, &state);
        get_object(g, g->player.now_loc.x, g->player.now_loc.y, &state);

        // show_enemis(g);
        load_player_detail(g, &state);


        if(state.quit) break;
        else if(state.ended_game) return STATE_PREGAME;
    }
    delwin(main_game);
    free_game(g);
    return STATE_END_GAME;
}