//
// Created by gonza on 11/06/2022.
//

#ifndef MARCELO_STARTING_FILES_AUTHENTICATION_H
#define MARCELO_STARTING_FILES_AUTHENTICATION_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define MAX_USER_PASS 500

typedef enum auth_level{
    ADMIN_AUTH_LEVEL = 0,
    USER_AUTH_LEVEL = 1,
} auth_level;

typedef struct user_pass {
    uint8_t * user;
    uint8_t * password;
    uint8_t level;
} user_pass;

#endif
