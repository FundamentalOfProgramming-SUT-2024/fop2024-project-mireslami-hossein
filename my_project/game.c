#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ncursesw/ncurses.h>
#include "cjson/cJSON.h"
#include <regex.h>
#include <time.h>

#include "basic_loads.h"

// کمکی
bool is_empty_room(Level level, Room room, Loc loc){
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
        if(is_empty_room(level, room, res)) return res;
    }
    // res.x = rand_in(room.e1.x+1, room.e4.x-1);
    // res.y = rand_in(room.e1.y+1, room.e4.y-1);
    // return res;
}

void putch_map(WINDOW* win, Level* level, int y, int x, char ch, int color) {
    wattron(win, COLOR_PAIR(color));
    mvwaddch(win, y, x, ch);
    level->map[y][x] = ch;
    wattroff(win, COLOR_PAIR(color));
}

void put_icon_map(WINDOW* win, Level* level, int y, int x, char ch, char* icon, int color){
    wattron(win, COLOR_PAIR(color));
    mvwaddstr(win, y, x, icon);
    level->map[y][x] = ch;
    wattroff(win, COLOR_PAIR(color));
}

void draw_corridor_two_bends(WINDOW *win, Level* level, int x1, int y1, int x2, int y2, int mode) {
    if(mode == 1){
        int mid_x = (x1 + x2) / 2;

        int start_x = min(x1, mid_x);
        int end_x   = max(x1, mid_x);
        
        for (int x = start_x; x <= end_x; x++) {
            putch_map(win, level, y1, x, '#', ORANGE_TEXT);
        }
        
        int start_y = min(y1, y2);
        int end_y   = max(y1, y2);
        for (int y = start_y; y <= end_y; y++) {
            putch_map(win, level, y, mid_x, '#', ORANGE_TEXT);
        }
        
        start_x = min(mid_x, x2);
        end_x   = max(mid_x, x2);
        for (int x = start_x; x <= end_x; x++) {
            putch_map(win, level, y2, x, '#', ORANGE_TEXT);
        }
    } else {
        int mid_y = (y1 + y2) / 2;

        int start_y = min(y1, mid_y);
        int end_y   = max(y1, mid_y);
        
        for (int y = start_y; y <= end_y; y++) {
            putch_map(win, level, y, x1, '#', ORANGE_TEXT);
        }
        
        int start_x = min(x1, x2);
        int end_x   = max(x1, x2);
        for (int x = start_x; x <= end_x; x++) {
            putch_map(win, level, mid_y, x, '#', ORANGE_TEXT);
        }
        
        start_y = min(mid_y, y2);
        end_y   = max(mid_y, y2);
        for (int y = start_y; y <= end_y; y++) {
            putch_map(win, level, y, x2, '#', ORANGE_TEXT);
        }
    }

    wrefresh(win);
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

// تابع اصلی
void initialize_map(Game* g){
    Map* map = malloc(sizeof(Map));
    g->map = map;
    
    int levels_num = 4;
    map->levels_num = levels_num;
    map->main_levels = (Level*)malloc(levels_num * sizeof(Level));
    map->visited_levels = (Level*)malloc(levels_num * sizeof(Level));
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
            map->main_levels[i].visited[j] = (int*)malloc(width * sizeof(int));
        }
        map->main_levels[i].rooms_num = 8;
        
        Room* map_rooms = malloc(map->main_levels[i].rooms_num * sizeof(Room));
        map->main_levels[i].rooms = map_rooms;
    }
}

void connect_door_pairs(WINDOW *win, Level *level) {
    // دوری
    for(int i = 0; i < 3; i++){
        draw_corridor_two_bends(win, level, level->rooms[i].doors[1].loc.x,
                                    level->rooms[i].doors[1].loc.y,
                                    level->rooms[i+1].doors[0].loc.x,
                                    level->rooms[i+1].doors[0].loc.y, 1);
    }
    for(int i = 4; i < 7; i++){
        draw_corridor_two_bends(win, level, level->rooms[i].doors[1].loc.x,
                                    level->rooms[i].doors[1].loc.y,
                                    level->rooms[i+1].doors[0].loc.x,
                                    level->rooms[i+1].doors[0].loc.y, 1);
    }
    draw_corridor_two_bends(win, level, level->rooms[0].doors[0].loc.x,
                                    level->rooms[0].doors[0].loc.y,
                                    level->rooms[4].doors[0].loc.x,
                                    level->rooms[4].doors[0].loc.y, 2);
    
    draw_corridor_two_bends(win, level, level->rooms[7].doors[1].loc.x,
                                level->rooms[7].doors[1].loc.y,
                                level->rooms[3].doors[1].loc.x,
                                level->rooms[3].doors[1].loc.y, 2);

    // فرعی ها
    draw_corridor_two_bends(win, level, level->rooms[1].doors[2].loc.x,
                                    level->rooms[1].doors[2].loc.y,
                                    level->rooms[5].doors[2].loc.x,
                                    level->rooms[5].doors[2].loc.y, 2);
    draw_corridor_two_bends(win, level, level->rooms[2].doors[2].loc.x,
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

void draw_rooms_and_corridors(Game* g, WINDOW* win, int h, int w, Map* map, int level_num){
    // O: obstacle/ X: Trap
    
    Level* level = &map->main_levels[level_num];
    
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
    
    // Golden room
    int golden_room_i = find_good_id_for_room(map, level_num);
    level->rooms[golden_room_i].type = 2;

    // for each room
    for (int i = 0; i < level->rooms_num; i++){
        Room *r = &level->rooms[i];
        int color_id = WHITE_TEXT;
        if(r->type == 1) color_id = PURPLE_TEXT;

        if(level_num == 3 && r->type == 2){
            color_id = YELLOW_TEXT;
        }
        
        putch_map(win, level, r->e1.y, r->e1.x, '.', color_id);
        putch_map(win, level, r->e2.y, r->e2.x, '.', color_id);
        putch_map(win, level, r->e3.y, r->e3.x, '|', color_id);
        putch_map(win, level, r->e4.y, r->e4.x, '|', color_id);

        for (int j = r->e1.x + 1; j < r->e2.x; j++){
            putch_map(win, level, r->e1.y, j, '_', color_id);
            putch_map(win, level, r->e4.y, j, '_', color_id);
        }
        for (int j = r->e1.y + 1; j < r->e3.y; j++){
            putch_map(win, level, j, r->e1.x, '|', color_id);
            putch_map(win, level, j, r->e4.x, '|', color_id);
        }
        for (int j = r->e1.y + 1; j < r->e3.y; j++){
            for (int z = r->e1.x + 1; z < r->e2.x; z++){
                putch_map(win, level, j, z, '.', color_id);
            }
        }
        
        // //window
        // r->window.x = rand_in(r->e1.x + 1, r->e2.x - 1);
        // r->window.y = (rand() % 2 == 0) ? r->e1.y : r->e4.y;
        // putch_map(win, level, r->window.y, r->window.x, '=');

        add_doors_to_room(i,r, level->mode);
    }
    connect_door_pairs(win, level);
    
    // for each room
    for (int i = 0; i < level->rooms_num; i++){
        Room *r = &level->rooms[i];
        // adding doors
        int color_id = WHITE_TEXT;
        if(r->type == 1) color_id = PURPLE_TEXT;
        
        for (int d = 0; d < 4; d++){
            if (r->doors[d].loc.x > 0 && r->doors[d].loc.y > 0){
                if(r->doors[d].type == 1){
                    putch_map(win, level, r->doors[d].loc.y, r->doors[d].loc.x, '?', color_id);
                } else{
                    putch_map(win, level, r->doors[d].loc.y, r->doors[d].loc.x, '+', color_id);
                }
            }
        }

        //O
        r->O.x = rand_in(r->e1.x + 2, r->e2.x - 2);
        r->O.y = rand_in(r->e1.y + 2, r->e4.y - 2);
        putch_map(win, level, r->O.y, r->O.x, 'O', color_id);
        
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
            putch_map(win, level, r->traps[j].loc.y, r->traps[j].loc.x, 'X', RED_TEXT);
        }
        // golds
        int golds_num, rand_num;
        if(r->type == 0){
            golds_num = rand_in(1 + r->w/5 + r->h/5, 3 + r->w/5 + r->h/5);
            
        } else if (r -> type == 1){
            golds_num = rand_in(1 + r->w/5 + r->h/5, 3 + r->w/5 + r->h/5);
        } else if(r -> type == 2){
            golds_num = rand_in(2, 3 + r->w/5 + r->h/5);
        }
        for(int j = 0; j < golds_num; j ++){
            r->golds[j].loc = find_empty_place_room(*level, *r, 1);
            rand_num = rand_in(1, 10);
            if(rand_num <= 8){
                r->golds[j].type = 0;
                put_icon_map(win, level, r->golds[j].loc.y, r->golds[j].loc.x, '$', "\u26c2",  YELLOW_TEXT);
            } else{
                r->golds[j].type = 1;
                put_icon_map(win, level, r->golds[j].loc.y, r->golds[j].loc.x, '$', "\u26c2",  BLACK_TEXT);
            }
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
                put_icon_map(win, level, r->foods[j].loc.y, r->foods[j].loc.x, 'F', "\u2299",  LIGHT_GREEN_TEXT);
            } else if (rand_num <= 65){ //magic
                r->foods[j].type = 1;
                r->foods[j].HP = 50;
                r->foods[j].speed = 20;
                r->foods[j].time = 20;
                put_icon_map(win, level, r->foods[j].loc.y, r->foods[j].loc.x, 'F', "\u2299",  CYAN_TEXT);
            }else if (rand_num <= 80){ // perfect
                r->foods[j].type = 2;
                r->foods[j].HP = 50;
                r->foods[j].power = 20;
                r->foods[j].time = 15;
                put_icon_map(win, level, r->foods[j].loc.y, r->foods[j].loc.x, 'F', "\u2299",  YELLOW_TEXT);
            } else { // Bad
                r->foods[j].type = 3;
                r->foods[j].HP = -50;
                r->foods[j].time = 0;
                put_icon_map(win, level, r->foods[j].loc.y, r->foods[j].loc.x, 'F', "\u2299",  RED_TEXT);
            }
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
                    put_icon_map(win, level, r->enchants[j].loc.y, r->enchants[j].loc.x, 'E', "\u2695",  PINK_TEXT);
                } else if (rand_num == 1){
                    r->enchants[j].type = 1;
                    r->enchants[j].speed = 2;
                    put_icon_map(win, level, r->enchants[j].loc.y, r->enchants[j].loc.x, 'E', "\u26f7",  PINK_TEXT);
                } else if (rand_num == 2){
                    r->enchants[j].type = 2;
                    r->enchants[j].power = 2;
                    put_icon_map(win, level, r->enchants[j].loc.y, r->enchants[j].loc.x, 'E', "\u2620",  PINK_TEXT);
                }
                r->enchants[j].time = 10;
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
                put_icon_map(win, level, r->weapons[j].loc.y, r->weapons[j].loc.x, 'w', "⚔",  WHITE_TEXT);
            }else if (rand_num == 2){
                r->weapons[j].type = 2;
                r->weapons[j].num = 8;
                r->weapons[j].damage = 15;
                put_icon_map(win, level, r->weapons[j].loc.y, r->weapons[j].loc.x, 'w', "🪄",  WHITE_TEXT);
            } else if (rand_num == 3){
                r->weapons[j].type = 3;
                r->weapons[j].num = 5;
                r->weapons[j].damage = 5;
                put_icon_map(win, level, r->weapons[j].loc.y, r->weapons[j].loc.x, 'w', "➳",  WHITE_TEXT);
            } else {
                r->weapons[j].type = 4;
                r->weapons[j].num = 1;
                r->weapons[j].damage = 10;
                put_icon_map(win, level, r->weapons[j].loc.y, r->weapons[j].loc.x, 'w', "🗡",  WHITE_TEXT);
            }

        }
        // enenmies
        int enemy_num = rand_in(0, 1 + r->w/7 + r->h/7);
        if(r->type == 2){
            enemy_num = rand_in(4, 4 + r->w/7 + r->h/7);
        }
        for(int j = 0; j < enemy_num; j ++){
            g->enemys[j].loc = find_empty_place_room(*level, *r, 1);

            rand_num = rand_in(0, 4);
            if (rand_num == 0){
                g->enemys[j].type = 0;
                g->enemys[j].damage = 5;
                g->enemys[j].hp = 5;
                g->enemys[j].possible_moves = 0;
                putch_map(win, level, r->traps[j].loc.y, r->traps[j].loc.x, 'D', ORANGE_TEXT);
            }else if (rand_num == 1){
                g->enemys[j].type = 1;
                g->enemys[j].damage = 5;
                g->enemys[j].hp = 10;
                g->enemys[j].possible_moves = 0;
                putch_map(win, level, r->traps[j].loc.y, r->traps[j].loc.x, 'F', ORANGE_TEXT);
            } else if (rand_num == 2){
                g->enemys[j].type = 2;
                g->enemys[j].damage = 5;
                g->enemys[j].hp = 15;
                g->enemys[j].possible_moves = 5;
                putch_map(win, level, r->traps[j].loc.y, r->traps[j].loc.x, 'G', ORANGE_TEXT);
            }else if (rand_num == 3){
                g->enemys[j].type = 3;
                g->enemys[j].damage = 5;
                g->enemys[j].hp = 20;
                g->enemys[j].possible_moves = 1000;
                putch_map(win, level, r->traps[j].loc.y, r->traps[j].loc.x, 'S', ORANGE_TEXT);
            } else {
                g->enemys[j].type = 4;
                g->enemys[j].damage = 5;
                g->enemys[j].hp = 30;
                g->enemys[j].possible_moves = 5;
                putch_map(win, level, r->traps[j].loc.y, r->traps[j].loc.x, 'U', ORANGE_TEXT);
            }
            
        }
    }

    // Stair
    // // To down   
    
    int stair_room_index, color_id;
    if(level_num == 3){
        stair_room_index = golden_room_i;
        color_id = GREEN_TEXT_YELLOW;
    } else{
        stair_room_index = find_good_id_for_room(map, level_num);
        color_id = GREEN_TEXT_WHITE;
    }
    level->stair_room_index = stair_room_index;
    Room* t_room = &level->rooms[stair_room_index];
    
    Stair* stair = &level->stairs[0];
    stair->level_num = level->number;
    stair->loc = find_empty_place_room(*level, *t_room,1);
    stair->is_down = TRUE;
    putch_map(win, level, stair->loc.y, stair->loc.x, '>', color_id);


    // // To up
    if(level_num != 0){
        Stair* stair_up = &level->stairs[1];
        stair_up->is_down = FALSE;
        stair_up->level_num = level_num;
        Room* last_room = map->main_levels[level_num - 1].rooms;
        stair_up->loc = map->main_levels[level_num - 1].stairs[0].loc;
        
        putch_map(win, level, stair_up->loc.y, stair_up->loc.x, '<', GREEN_TEXT_WHITE);
    }
    wrefresh(win);
}

void make_random_map(Game* g){
    int width = g->map->width, height = g->map->height;

    int x_w = 1;
    int y_w = LINES/2 - height/2;

    WINDOW* main_game = newwin(height, width, y_w, x_w);
    
    refresh();
    box(main_game, 0, 0);
    wrefresh(main_game);
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
        
        draw_rooms_and_corridors(g, main_game, height, width, g->map, j);
        getch();
        wclear(main_game);
    }


    delwin(main_game);
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


    for (int i = 0; i < map->levels_num; i++) {
        Level* level = &map->visited_levels[i];

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
    free(map->visited_levels);

    free(map);
    g->map = NULL;
}


void load_main_game(Game* g){
    clear();
    start_color();
    game_initalize();
    
    Player player;
    player.level = 1;
    player.hp = 100;
    player.hungriness = 0;
    player.points = 0;
    player.golds = 0;

    player.now_loc.x = 0;
    player.now_loc.y = 0;

    g->player = player;
    initialize_map(g);
    make_random_map(g);

    g->player.now_loc.x = g->map->main_levels[0].rooms[0].e1.x + 1;
    g->player.now_loc.y = g->map->main_levels[0].rooms[0].e1.y + 1;

    free_game(g);
    // while(1){
        
    //         // handle key -> move->f,g,orib?/menus->food(E), /map show(M)/quit menu(Q)
    //         // load player loc
    //         // giving objects in this loc -> delete from map, add to player
            
    //         // show visited parts
    //         // show visible area (if in corridor)
    // }
}