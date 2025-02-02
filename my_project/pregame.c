#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ncurses.h>
#include "cjson/cJSON.h"
#include <regex.h>
#include <time.h>

#include "basic_loads.h"

char pregame_user_options[5][50] = {
    "New Game", "Load Game", "Leaderboard", "Settings" , "Profile"
};
char pregame_guest_options[4][50] = {
    "New Game", "Leaderboard", "Settings" , "Profile"
};

char lb_titles[6][20] = {
    "Rank", "Username", "Total Score","Total Golds", "Total Games", "Experience"
};
char setting_lebels[2][50] = {
    "Player Color: ", "Game Hardness: "
};
char player_colors[3][20] = {
    "White", "Green", "Cyan"
};
int player_color_ids[3] = {
    COLOR_WHITE, CUSTOM_GREEN, CUSTOM_CYAN
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
    switch (mode) {
        case 2:
            wattron(w, A_BOLD);
            break;
        case 3:
            wattron(w, A_BOLD |COLOR_PAIR(TEXT_COLOR));
            break;
    }

    mvwprintw(w, y, x_start, "%d", user.rank);
    mvwprintw(w, y, x_start + 14 - strlen(user.username)/2, "%s", user.username);
    mvwprintw(w, y, x_start + 30 - (int)log10(user.points + 1)/2, "%d", user.points);
    mvwprintw(w, y, x_start + 45 - (int)log10(user.ended_games + 1)/2, "%d", user.ended_games);
    mvwprintw(w, y, x_start + 58, "%ld", user.experience);

    switch (mode) {
        case 2:
            wattroff(w, A_BOLD);
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
            if(!user.is_guest && strcmp(t_user_data.username, user.username) == 0)
                mode = 2;

            t_user_data.points = cJSON_GetObjectItem(t_user, "points")->valueint;
            t_user_data.golds = cJSON_GetObjectItem(t_user, "golds")->valueint;
            t_user_data.ended_games = cJSON_GetObjectItem(t_user, "ended_games")->valueint;
            t_user_data.experience = cJSON_GetObjectItem(t_user, "experience")->valueint;
            
            if(i <= 2)
                mode = 3;
            
            print_userdata(w, t_user_data, y_start + 2*(i - start_i), x_start, mode);
        }
    }


}

void show_leaderboard(User user){
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
        } else if(ch == 'q' || ch == 'Q') break;
    }

}


void show_setting(Player* player, Game* g){
    clear();
    int height = 18, width = 35;
    int y_w = LINES/2 - height/2;
    int x_w = COLS/2 - width/2;
    WINDOW* set_w = newwin(height , width,y_w,x_w);
    wrefresh(set_w);
    box(set_w, 0, 0);
    refresh();

    print_title(set_w, "Setting", 2, width/2);

    int start_y = 6, start_x = 18;
    print_messages(set_w, setting_lebels, 2, start_y, start_x, 'r', LABEL_COLOR, 4);
    get_player_color(set_w, width, height, start_y, start_x);

    getch();
}

void load_pregame_page(User* user, Player* player, Game* g){
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
    char ms[2][50] = {"Hint:", "Press Q anywhere to go back!"};
    int x = width/2;
    int y = 6;
    
    wattron(menu, COLOR_PAIR(TEXT_COLOR));
    wattron(menu, A_BOLD | A_ITALIC);
    mvwprintw(menu, y-2, 3,"%s", ms[0]);
    wattroff(menu, A_BOLD | A_ITALIC);
    mvwprintw(menu, y-2, 9,"%s", ms[1]);
    wattroff(menu, COLOR_PAIR(TEXT_COLOR));
    wrefresh(menu);
    if(user->is_guest){
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
    
    if(selected == 2 - user->is_guest){
        show_leaderboard(*user);
    }else if(selected == 3 - user->is_guest){
        show_setting(player, g);
    }

    delwin(menu);
}