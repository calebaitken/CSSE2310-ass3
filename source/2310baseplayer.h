//
// Created by caleb on 2019-09-12.
//

#ifndef ASS3_2310ALICE_H
#define ASS3_2310ALICE_H

#include "2310shared.h"

#define BUFFER_SIZE 255
#define MAX_INSTRUCTION_LEN 8

typedef enum {
    OK = 0,
    BADARGNUM = 1,
    BADPLAYERNUM = 2,
    BADPOSITION = 3,
    BADTHRESHOLD = 4,
    BADHANDSIZE = 5,
    BADMESSAGE = 6,
    UNEXPECTEDEOF = 7
} Status;

typedef struct {
    int playerCount;
    int position;
    int threshold;
    int handSize;
    int currentPlayer;
} GameStats;

typedef struct {
    char* type;
    int argc;
    char** args;
} Instruction;

void init_player(char** argv, GameStats* gameStats);
void init_hand(int size, char** cards, Card** hand);

void get_instruction(Instruction* instruction);
void get_instruction_type(char* message, char** instructionType);
void get_hand_args(char* message, int* argc, char*** args);
void get_newround_args(char* message, int* argc, char*** args);
void get_played_args(char* message, int* argc, char*** args);

Card play_card(char lead, int count, Card* hand);

int main(int argc, char** argv);
void game_loop(GameStats game);

void quit_on_error(Status s);

#endif //ASS3_2310ALICE_H
