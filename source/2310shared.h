//
// Created by caleb on 2019-09-23.
//

#ifndef ASS3_SHARED_H
#define ASS3_SHARED_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/time.h>
#include <sys/types.h>
#include <signal.h>

typedef struct {
    char suit;
    char rank;
} Card;

bool valid_card(char suit, char rank);
bool valid_deck(Card* deck, int size);
bool valid_player_count(const char* playerCount);
bool valid_threshold(const char* threshold);
bool valid_hand_size(const char* handSize);
bool valid_position(const char* position, int playerCount);

#endif //ASS3_SHARED_H
