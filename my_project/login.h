#ifndef LOGIN_H
#define LOGIN_H

#include "states.h"

int signup_user(User* user);
int login_user(User* user);
int login_as_guest(User* user);

int load_first_page(User* user);

#endif