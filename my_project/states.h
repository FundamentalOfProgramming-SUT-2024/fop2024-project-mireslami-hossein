#ifndef STATES_H
#define STATES_H

typedef enum {
    STATE_LOGIN,
    STATE_PREGAME,
    STATE_GAME,
    STATE_END_GAME,
    STATE_EXIT
} GameState;

#endif