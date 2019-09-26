//
// Created by Caleb on 2019-09-10.
//

#ifndef ASS3_2310HUB_H
#define ASS3_2310HUB_H

#include "2310shared.h"

#define BUFFER_SIZE 255

typedef enum {
    OK = 0,
    BADARGNUM = 1,
    BADTHRESHOLD = 2,
    DECKERROR = 3,
    BADCARDNUM = 4,
    PLAYERERROR = 5,
    PLAYEREOF = 6,
    BADMSG = 7,
    BADCARD = 8,
    SSIGHUP = 9
} Status;

typedef struct {
    int count;
    int used;
    Card* cards;
} Deck;

typedef struct {
    Deck deck;
    int threshold;
    int playerCount;
    int numRounds;
} Game;

void init_game(int playerCount, char** argv, Game* game);
void init_deck(const char* deckName, Deck* deck);
void init_threshold(const char* thresholdArg, int* threshold);
int* init_players(Game game, char** argv, int*** playerPipes);

void assign_hands(Deck deck, int handSize,
        int playerCount, int*** playerPipes);

void start_round(int leadPlayer, int playerCount, int*** playerPipes);
Card get_play(int currentPlayer, int*** playerPipes);
void print_move(int currentPlayer, Card card,
        int playerCount, int*** playerPipes);
int find_winner(int playerCount, Card* cardsPlayed);
int count_d_cards(int playerCount, Card* cardsPlayed);
void print_scores(int playerCount, int threshold, int* scores, int* dCards);
void gameover(int playerCount, int*** playerPipes);

int main(int argc, char** argv);
void game_loop(Game game, int*** playerPipes);

int create_player_process(char** args, int childRead[2], int childWrite[2]);

bool verify_players(int playerCount, int*** playerPipes);

void handle_sighup(int sig);
void quit_on_error(Status s);

#endif //ASS3_2310HUB_H
