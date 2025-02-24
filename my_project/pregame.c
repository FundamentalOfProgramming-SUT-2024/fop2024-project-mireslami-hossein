#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ncurses.h>
#include "cjson/cJSON.h"
#include <regex.h>
#include <time.h>
#include <locale.h>

#include "basic_loads.h"

#include "pregame.h"


char pregame_user_options[5][50] = {
    "New Game", "Load Game", "Leaderboard", "Settings" , "Profile"
};
char pregame_guest_options[4][50] = {
    "New Game", "Leaderboard", "Settings" , "Profile"
};

char lb_titles[6][20] = {
    "Rank", "Username","Total Golds", "Total Score", "Total Games", "Experience"
};
char setting_lebels[2][50] = {
    "Player Color: ", "Game Hardness: "
};
char player_colors[3][20] = {
    "White", "Green", "Cyan"
};
int player_color_ids[3] = {
    WHITE_TEXT, GREEN_TEXT, CYAN_TEXT
};

// Leaderboard
typedef struct {
    cJSON* user;
    int points;
    int original_index;
} UserData;

int compare_users(const void* a, const void* b) {
    UserData* userA = (UserData*)a;
    UserData* userB = (UserData*)b;
    return userB->points - userA->points;
}

bool grade_users(UserData** users_sorted){
    char* users_data = read_file("data/users.json");
    
    cJSON* root = cJSON_Parse(users_data);
    free(users_data);
    if(!root){
        return FALSE;
    }
    int c = count_users();
    cJSON* users = cJSON_GetObjectItem(root, "users");

    UserData* user_list = (UserData*)malloc(c * sizeof(UserData));

    for (int i = 0; i < c; i++) {
        cJSON* user = cJSON_GetArrayItem(users, i);
        cJSON* points = cJSON_GetObjectItem(user, "points");
        user_list[i].user = user;
        user_list[i].points = points ? points->valueint : 0;
        user_list[i].original_index = i;
    }

    qsort(user_list, c, sizeof(UserData), compare_users);

    // set ranks to users
    for (int i = 0; i < c; i++) {
        cJSON* user = user_list[i].user;
        cJSON* rank_field = cJSON_GetObjectItem(user, "rank");

        cJSON_SetNumberValue(rank_field, i + 1);
    }
    *users_sorted = user_list;

    char* new_data = cJSON_Print(root);
    write_file("data/users.json", new_data);
    
    // cJSON_Delete(root);
    return TRUE;
}

void print_headers(WINDOW* w, char lb_titles[6][20], int y, int x){
    wattron(w, A_BOLD | COLOR_PAIR(LABEL_COLOR));
    mvwprintw(w, y, x, "%s", lb_titles[0]);
    mvwprintw(w, y, x + 12, "%s", lb_titles[1]);
    mvwprintw(w, y, x + 28, "%s", lb_titles[2]);
    mvwprintw(w, y, x + 42, "%s", lb_titles[3]);
    mvwprintw(w, y, x + 56, "%s", lb_titles[4]);
    mvwprintw(w, y, x + 70, "%s", lb_titles[5]);
    wattroff(w, A_BOLD | COLOR_PAIR(LABEL_COLOR));
    wrefresh(w);
    refresh();
}
// mode: 1:default 2:User 3:Top Users
void print_userdata(WINDOW* w, User user, int y, int x_start, int mode){
    // mvprintw(0,0, "mode: %d username: %s", mode, user.username);
    // getch();
    // refresh();
    switch (mode) {
        case 2:
            wattron(w, A_BOLD | COLOR_PAIR(ORANGE_TEXT));
            break;
        case 3:
            wattron(w, A_BOLD |COLOR_PAIR(TEXT_COLOR));
            break;
    }
    if(user.rank == 1){
        mvwprintw(w, y, x_start-3, "🥇 ");
    } else if(user.rank == 2){
        mvwprintw(w, y, x_start-3, "🥈 ");
    } else if(user.rank == 3){
        mvwprintw(w, y, x_start-3, "🥉 ");
    } else{
        mvwprintw(w, y, x_start-3, "  ");
    }
    mvwprintw(w, y, x_start, "%d", user.rank);
    mvwprintw(w, y, x_start + 14 - strlen(user.username)/2, "%s", user.username);
    mvwprintw(w, y, x_start + 30 - (int)log10(user.points + 1)/2, "%d", user.golds);
    mvwprintw(w, y, x_start + 45 - (int)log10(user.points + 1)/2, "%d", user.points);
    mvwprintw(w, y, x_start + 58 - (int)log10(user.ended_games + 1)/2, "%d", user.ended_games);
    mvwprintw(w, y, x_start + 73, "%ld", user.experience);
    

    switch (mode) {
        case 2:
            wattroff(w, A_BOLD | COLOR_PAIR(ORANGE_TEXT));
            break;
        case 3:
            wattroff(w, A_BOLD |COLOR_PAIR(TEXT_COLOR));
            break;
    }
    
    wrefresh(w);
    refresh();
}

void print_users(WINDOW* w, User user, int height, int width, int start_i, int max, int y_start, int x_start){
    clear_part(w, y_start, x_start, y_start + 12 , width - 2);

    UserData* users_sorted;

    if(!grade_users(&users_sorted)){
        char* ms = "Not Any Users Yet ...";
        wattron(w, COLOR_PAIR(TEXT_COLOR));
        mvwprintw(w, (height - y_start)/2 + y_start/2, width/2 - strlen(ms)/2, "%s", ms);
        wattroff(w, COLOR_PAIR(TEXT_COLOR));
        wrefresh(w);
    } else{
        int c = count_users();
        for (int i = start_i; i < min(max,c) + start_i; i++) {
            int mode = 1;

            User t_user_data;
            reset_user_data(&t_user_data);

            cJSON* t_user = users_sorted[i].user;
            
            t_user_data.rank = cJSON_GetObjectItem(t_user, "rank")->valueint;

            strcpy(t_user_data.username, cJSON_GetObjectItem(t_user, "username")->valuestring);

            if(strcmp(t_user_data.username, user.username) == 0)
                mode = 2;
            else if(i <= 2)
                mode = 3;

            char username[MAX_USERNAME];
            char rand_names[5][10] = {"Goat ", "Lion ", "Hero ", "Heroic ", "Gallant "};
            if(t_user_data.rank <= 3){
                strcpy(username, rand_names[rand()%5]);
                strcat(username, t_user_data.username);
                strcpy(t_user_data.username ,username);
            }
            t_user_data.points = cJSON_GetObjectItem(t_user, "points")->valueint;
            t_user_data.golds = cJSON_GetObjectItem(t_user, "golds")->valueint;
            t_user_data.ended_games = cJSON_GetObjectItem(t_user, "ended_games")->valueint;
            t_user_data.experience = cJSON_GetObjectItem(t_user, "experience")->valueint;
            
            
            print_userdata(w, t_user_data, y_start + 2*(i - start_i), x_start, mode);
        }
    }


}

void print_colors(WINDOW* w, int width, int height, int y, int x, int color_num, int selected){
    
    for(int i = 0; i < color_num; i++){
        wattron(w, COLOR_PAIR(player_color_ids[i]) | A_BOLD);
        if(selected == i){
            wattron(w, A_REVERSE);
            mvwprintw(w, y, x, "%s", player_colors[i]);
            wattroff(w, A_REVERSE | A_BOLD);
        } else{
            mvwprintw(w, y, x, "%s", player_colors[i]);
            wattroff(w, COLOR_PAIR(player_color_ids[i]) | A_BOLD);
        }

        x += (strlen(player_colors[i]) + 5);
    }
    wrefresh(w);
    refresh();
}


bool show_leaderboard(User user){
    clear();
    int height = 22, width = 90;
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
    print_headers(lb_table, lb_titles, y_start, x_start);

    keypad(lb_table, TRUE);
    int ch, start_index = 0, num_of_users = count_users(), max = 6;
    while(1){
        print_users(lb_table, user, height, width, start_index, max, y_start + 2, x_start + 2);
        ch = getch();
        if(ch == KEY_UP){
            if(start_index > 0) --start_index;
        }else if(ch == KEY_DOWN){
            if(start_index < num_of_users - max) ++start_index;
        } else if(ch == KEY_ESC){
            return FALSE;
        }
    }
    return TRUE;
}

void get_game_hardness(Game* g, WINDOW* set_w) {
    int hardness = (g->hardness > 0 && g->hardness <= 10) ? g->hardness : 5;
    int ch;
    int confirmed = 0;

    int y_hard = 12, x_hard = 0;
    keypad(set_w, TRUE);
    
    while (confirmed == 0) {

        for (int i = y_hard; i < y_hard + 3; i++) {
            mvwprintw(set_w, i, 1, "%*s", 68, " ");
        }
        mvwprintw(set_w, y_hard, (70 - strlen("Game Hardness (1-10):"))/2, "Game Hardness (0-10):");
        mvwprintw(set_w, y_hard+1, (70 - 2)/2, "%d", hardness);
        mvwprintw(set_w, y_hard+2, (70 - strlen("Up/Down: adjust, Enter: Confirm"))/2, "Up/Down: adjust, Enter: Confirm");
        wrefresh(set_w);
        
        ch = wgetch(set_w);
        if (ch == KEY_UP) {
            if (hardness < 10)
                hardness++;
        } else if (ch == KEY_DOWN) {
            if (hardness > 1)
                hardness--;
        } else if (ch == '\n') {
            confirmed = 1;
        }
    }
    
    if (confirmed == 1)
        g->hardness = hardness;
}

bool show_setting(Game* g){
    clear();
    int height = 18, width = 70;
    int y_w = LINES/2 - height/2;
    int x_w = COLS/2 - width/2;
    WINDOW* set_w = newwin(height , width,y_w,x_w);
    wrefresh(set_w);
    box(set_w, 0, 0);
    refresh();

    print_title(set_w, "Setting", 2, width/2);

    int start_y = 6, start_x = 18;
    print_messages(set_w, setting_lebels, 2, start_y, start_x, 'r', LABEL_COLOR, 4);
    int selected = 0;
    switch(g->user->color_id){
        case WHITE_TEXT:
            selected = 0;
            break;
        case GREEN_TEXT:
            selected = 1;
            break;
        case CYAN_TEXT:
            selected = 2;
            break;
    }
    int pressed = 0;
    int color_num = 3;
    while(pressed == 0){
        print_colors(set_w, width, height, start_y + 2, start_x + 10, color_num, selected);
        int ch = getch();
        if(ch == KEY_LEFT){
            if(selected > 0) --selected;
        }else if(ch == KEY_RIGHT){
            if(selected < color_num-1) ++selected;
        } else if(ch == KEY_ESC){ // ESC
            return FALSE;
        }
        else if(ch == '\n') pressed = 1;
    }
    if(pressed == 1){
        g->user->color_id = player_color_ids[selected];
        if(!g->user->is_guest) update_user_int_data(*(g->user), "color_id", g->user->color_id);
        get_game_hardness(g, set_w);
    }

    return FALSE;
}

bool show_profile(User *user) {
    clear();
    int height = 19;
    int width = 60;
    int start_y = (LINES - height) / 2;
    int start_x = (COLS - width) / 2;
    WINDOW *profile_win = newwin(height, width, start_y, start_x);
    box(profile_win, 0, 0);
    wattron(profile_win, A_BOLD | COLOR_PAIR(HEADER_COLOR));
    mvwprintw(profile_win, 2, (width - strlen("User Profile")) / 2, "User Profile");
    wattroff(profile_win, A_BOLD | COLOR_PAIR(HEADER_COLOR));
    int base = 4, spacing = 2;
    wattron(profile_win, COLOR_PAIR(LABEL_COLOR) | A_BOLD);
    mvwprintw(profile_win, base, 5, "Username:");
    mvwprintw(profile_win, base + spacing, 5, "Email:");
    mvwprintw(profile_win, base + 2 * spacing, 5, "Rank:");
    mvwprintw(profile_win, base + 3 * spacing, 5, "Points:");
    mvwprintw(profile_win, base + 4 * spacing, 5, "Golds:");
    mvwprintw(profile_win, base + 5 * spacing, 5, "Ended Games:");
    mvwprintw(profile_win, base + 6 * spacing, 5, "Last GameID:");
    wattroff(profile_win, COLOR_PAIR(LABEL_COLOR) | A_BOLD);
    wattron(profile_win, COLOR_PAIR(WHITE_TEXT));
    if(user->is_guest) {
        mvwprintw(profile_win, base, 23, "You are a guest!");
        mvwprintw(profile_win, base + spacing, 23, "guest");
    } else {
        mvwprintw(profile_win, base, 23, "%s", user->username);
        mvwprintw(profile_win, base + spacing, 23, "%s", user->email);
    }
    mvwprintw(profile_win, base + 2 * spacing, 23, "%d", user->rank);
    mvwprintw(profile_win, base + 3 * spacing, 23, "%d", user->points);
    mvwprintw(profile_win, base + 4 * spacing, 23, "%d", user->golds);
    mvwprintw(profile_win, base + 5 * spacing, 23, "%d", user->ended_games);
    if(user->last_game_id == -1)
        mvwprintw(profile_win, base + 6 * spacing, 23, "You Haven't Played Yet!");
    else
        mvwprintw(profile_win, base + 6 * spacing, 23, "%d", user->last_game_id);
    wattroff(profile_win, COLOR_PAIR(WHITE_TEXT));
    refresh();
    wrefresh(profile_win);
    delwin(profile_win);
    refresh();

    while(1){
        int c = getch();
        if(c == KEY_ESC){
            return FALSE;
        }
    }
}

int load_pregame_page(Game* g){
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
    if(g->user->is_guest){
        while(!pressed){
            print_buttons(menu, pregame_guest_options, 4, selected, y, x, 2);
            handle_selected_btn(&selected, 4, &pressed);
        }
    } else{
        while(!pressed){
            print_buttons(menu, pregame_user_options, 5, selected, y, x, 2);
            handle_selected_btn(&selected, 5, &pressed);
        }
    }
    if(pressed == -1){
        return STATE_LOGIN;
    } else if(pressed == -2){ // ESC to go back
        return STATE_LOGIN;
    }

    bool state = TRUE;
    if(selected == 0){ // New game
        return STATE_GAME;
    }else if(selected == 2 - g->user->is_guest){
        state = show_leaderboard(*(g->user));
    } else if(selected == 3 - g->user->is_guest){
        state = show_setting(g);
    } else if(selected == 4 - g->user->is_guest){
        state = show_profile(g->user);
    }
    return STATE_PREGAME;
}