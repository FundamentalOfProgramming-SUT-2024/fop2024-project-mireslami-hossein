#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ncurses.h>
#include "cjson/cJSON.h"
#include <regex.h>
#include <time.h>

#include "basic_loads.h"

bool is_empty_room(Room room, Loc loc){
    if(loc.x == room.O.x && loc.y == room.O.y) return FALSE;
    
    return TRUE;
}

Loc find_empty_place_room(Room room){
    Loc res;
    while(1){
        res.x = rand_in(room.e1.x+1, room.e4.x-1);
        res.y = rand_in(room.e1.y+1, room.e4.y-1);

        if(is_empty_room(room, res)) return res;
    }
}

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

// کمکی
void putch_map(WINDOW* win, Level* level, int y, int x, char ch, int color) {
    wattron(win, COLOR_PAIR(color));
    mvwaddch(win, y, x, ch);
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


// تابع اصلی
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
        // case 3:
        //     room->doors[2].loc.x = room->e3.x + 1;
        //     room->doors[2].loc.y = room->e4.y;
        //     break;
        case 5: case 6:
            room->doors[2].loc.x = rand_in(room->e1.x + 2 ,room->e4.x - 2);
            room->doors[2].loc.y = room->e1.y;
            
            // room->doors[3].loc.x = rand_in(room->e1.x + 3 ,room->e4.x - 3);
            // room->doors[3].loc.y = room->e1.y;
            break;
        // case 6:
        //     room->doors[2].loc.x = rand_in(room->e1.x + 3 ,room->e4.x - 2);
        //     room->doors[2].loc.y = room->e1.y;
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

void draw_rooms_and_corridors(WINDOW* win, int h, int w, Map* map, int i){
    // for each room
    Level* level = &map->main_levels[i];
    
    level->mode = rand_in(1,5); //mode 1: none hidden / mode 2: 2 anchent / mode 3: 5 anchent
                                // mode 4: 6 / mode 5: 7
    for (int i = 0; i < level->rooms_num; i++){
        Room *r = &level->rooms[i];
        int color_id = WHITE_TEXT;
        if(level->mode == 2 && i == 2){
            color_id = PURPLE_TEXT;
        }else if(level->mode == 3 && i == 5){
            color_id = PURPLE_TEXT;
        }else if(level->mode == 4 && i == 6){
            color_id = PURPLE_TEXT;
        }else if(level->mode == 5 && i == 7){
            color_id = PURPLE_TEXT;
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
                putch_map(win, level, j, z, '.', GREEN_TEXT);
            }
        }

        // //window
        // r->window.x = rand_in(r->e1.x + 1, r->e2.x - 1);
        // r->window.y = (rand() % 2 == 0) ? r->e1.y : r->e4.y;
        // putch_map(win, level, r->window.y, r->window.x, '=');


        add_doors_to_room(i,r, level->mode);
    }
    connect_door_pairs(win, level);
    for (int i = 0; i < level->rooms_num; i++){
        Room *r = &level->rooms[i];
        // adding doors
        int color_id = WHITE_TEXT;
        if(level->mode == 2 && i == 2){
            color_id = PURPLE_TEXT;
        }else if(level->mode == 3 && i == 5){
            color_id = PURPLE_TEXT;
        }else if(level->mode == 4 && i == 6){
            color_id = PURPLE_TEXT;
        }else if(level->mode == 5 && i == 7){
            color_id = PURPLE_TEXT;
        }
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
        putch_map(win, level, r->O.y, r->O.x, 'O', GREEN_TEXT);
        
    }

    // Stair
    int room_num = rand_in(1, level->rooms_num);
    Room t_room = level->rooms[room_num];
    Stair stair;
    stair.level_num = level->number;
    stair.loc = find_empty_place_room(t_room);
    // trap (hidden)
    // foods
    // golds
    // 
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
        if(j == 1){
            int rooms_num = g->map->main_levels[j].rooms_num;
            Room* map_rooms = g->map->main_levels[j].rooms;
            for (int i = 0; i < rooms_num; i++){
                int x_s = (width / 4) * (i % 4) + 1;
                int x_f = (width / 4) * ((i % 4) + 1) - 1;
                int y_s = (height / 2) * (i / 4) + 1;
                int y_f = (height / 2) * ((i / 4) + 1) - 1;
                
                int w_room = rand_in(7, 10);
                int h_room = rand_in(5, 9);
                
                map_rooms[i].e1.x = rand_in(x_s, x_f - w_room - 1);
                map_rooms[i].e1.y = rand_in(y_s, y_f - h_room - 1);
                map_rooms[i].w = w_room;
                map_rooms[i].h = h_room;
                
                map_rooms[i].e2.x = map_rooms[i].e1.x + w_room;
                map_rooms[i].e2.y = map_rooms[i].e1.y;
                
                map_rooms[i].e3.x = map_rooms[i].e1.x;
                map_rooms[i].e3.y = map_rooms[i].e1.y + h_room;
                
                map_rooms[i].e4.x = map_rooms[i].e1.x + w_room;
                map_rooms[i].e4.y = map_rooms[i].e1.y + h_room;
                
                map_rooms[i].window.x = map_rooms[i].window.y = 0;
                map_rooms[i].O.x = map_rooms[i].O.y = 0;
                map_rooms[i].is_visited = false;
            }
            
            draw_rooms_and_corridors(main_game, height, width, g->map, j);
        }
    }

    getch();
    delwin(main_game);
}


void load_main_game(Game* g){
    clear();

    start_color();
    game_initalize();
    
    Player player;
    player.level = 1;
    player.hp = 100;
    player.hungriness = 0;
    player.now_loc.x = 0;
    player.now_loc.y = 0;

    g->player = player;
    initialize_map(g);
    make_random_map(g);

    g->player.now_loc.x = g->map->main_levels[0].rooms[0].e1.x + 1;
    g->player.now_loc.y = g->map->main_levels[0].rooms[0].e1.y + 1;

    // while(1){
        
    //         // handle key -> move->f,g,orib?/menus->food(E), /map show(M)/quit menu(Q)
    //         // load player loc
    //         // giving objects in this loc
            
    //         // show visited parts
    //         // show visible area (if in corridor)
    // }
}