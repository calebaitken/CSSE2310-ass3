//
// Created by caleb on 2019-09-12.
//
/*
 * EXIT CONDITION                               MESSAGE
 * 0    Normal exit
 * 1    Incorrect number of arguments           Usage: player players myid
 *                                              threshold handsize
 * 2    Number of players < 2 or not a number   Invalid players
 * 3    Invalid position for number of players  Invalid position
 * 4    Threshold < 2 or not a number           Invalid theshold
 * 5    Hand size < 1 or not a number           Invalid hand size
 * 6    Invalid message from hub                Invalid message
 * 7    Unexpected EOF from hub                 EOF
*/

#include "2310baseplayer.h"

/**
 * inits the player and checks args
 *
 * @param argv      args sent to player
 * @param gameStats game to init values into
 */
void init_player(char** argv, GameStats* gameStats) {
    int playerCount, position, threshold, handSize;

    if (!valid_player_count(argv[1])) {
        quit_on_error(BADPLAYERNUM);
    }

    playerCount = strtol(argv[1], NULL, 10);

    if (!valid_position(argv[2], playerCount)) {
        quit_on_error(BADPOSITION);
    }

    position = strtol(argv[2], NULL, 10);

    if (!valid_threshold(argv[3])) {
        quit_on_error(BADTHRESHOLD);
    }

    threshold = strtol(argv[3], NULL, 10);

    if (!valid_hand_size(argv[4])) {
        quit_on_error(BADHANDSIZE);
    }

    handSize = strtol(argv[4], NULL, 10);

    gameStats->playerCount = playerCount;
    gameStats->position = position;
    gameStats->handSize = handSize;
    gameStats->threshold = threshold;
}

/**
 * inits ahnd and checks values
 *
 * @param size      number of cards in hand
 * @param cards     array of char[2] cards to get values from
 * @param hand      pointer to list of cards to init into
 */
void init_hand(int size, char** cards, Card** hand) {
    for (int i = 0; i < size; i++) {
        if (!valid_card(cards[i][0], cards[i][1])) {
            quit_on_error(BADMESSAGE);
        }
        (*hand)[i].suit = cards[i][0];
        (*hand)[i].rank = cards[i][1];
    }
}

/**
 * gets instruction from stdin and verifies
 *
 * @param instruction   pointer to instruction to put values into
 */
void get_instruction(Instruction* instruction) {
    char buffer[BUFFER_SIZE];

    char* rs = fgets(buffer, BUFFER_SIZE, stdin);
    if (rs == NULL) {
        quit_on_error(UNEXPECTEDEOF);
    }
    if (buffer[strlen(rs) - 1] == '\n') {
        buffer[strlen(rs) - 1] = 0;
    } else {
        fclose(stdin);
    }

    char* type = calloc(MAX_INSTRUCTION_LEN, sizeof(char));
    get_instruction_type(buffer, &type);

    if (strcmp(type, "") == 0) {
        quit_on_error(BADMESSAGE);
    }

    int argc;
    char** args;
    if (strcmp(type, "HAND") == 0) {
        get_hand_args(buffer, &argc, &args);
    } else if (strcmp(type, "NEWROUND") == 0) {
        get_newround_args(buffer, &argc, &args);
    } else if (strcmp(type, "PLAYED") == 0) {
        get_played_args(buffer, &argc, &args);
    } else if (strcmp(type, "GAMEOVER") == 0) {
        quit_on_error(OK); // not actually error lul
    } else {
        quit_on_error(BADMESSAGE); // this shouldn't even be possible to reach
    }

    instruction->type = type;
    instruction->argc = argc;
    instruction->args = args;
}

/**
 * gets the type of instruction received from stdin
 *
 * @param message           char array recieved
 * @param instructionType   pointer to char array to put values into
 */
void get_instruction_type(char* message, char** instructionType) {
    const char* messages[] = {"HAND",
            "NEWROUND",
            "PLAYED",
            "GAMEOVER"};
    char* buffer = calloc(strlen(message), sizeof(char));
    for (int i = 0; i < 4; i++) {
        strcpy(buffer, message);
        buffer[strlen(messages[i])] = 0;
        buffer[strlen(message)] = 0;
        if (strcmp(buffer, messages[i]) == 0) {
            strcpy(*instructionType, buffer);
            return;
        }
    }
    strcpy(*instructionType, "");
}

/**
 * gets args from message received for a HAND type instruction
 *
 * @param message   message received from stdin
 * @param argc      pointer to int of number of agrs to put values into
 * @param args      pointer to array of char arrys of argv to put values into
 */
void get_hand_args(char* message, int* argc, char*** args) {
    char* buffer = calloc(strlen(message), sizeof(char));
    strcpy(buffer, message);
    for (int i = 0; i < strlen(buffer); i++) {
        if (buffer[i] == ',') {
            buffer[i] = 0;
            break;
        }
    }

    memmove(buffer, buffer + strlen("HAND"), strlen(buffer));

    if (strcmp(buffer, "") == 0) {
        quit_on_error(BADMESSAGE);
    }

    for (int i = 0; i < strlen(buffer); i++) {
        if (!isdigit((int) buffer[i])) {
            quit_on_error(BADMESSAGE);
        }
    }

    if (strlen(message) != (5 + (3 * strtol(buffer, NULL, 10)))) {
        quit_on_error(BADMESSAGE);
    }

    *args = calloc(strtol(buffer, NULL, 10), sizeof(char*));
    for (int i = 0; i < strtol(buffer, NULL, 10); i++) {
        if (8 + (3 * i) > strlen(message)) {
            quit_on_error(BADMESSAGE);
        }

        (*args)[i] = calloc(2, sizeof(char));
        (*args)[i][0] = message[6 + (3 * i)];
        (*args)[i][1] = message[7 + (3 * i)];
        if (!valid_card((*args)[i][0], (*args)[i][1])) {
            quit_on_error(BADMESSAGE);
        }
    }

    *argc = strtol(buffer, NULL, 10);
}

/**
 * gets args from message received for a NEWROUND type instruction
 *
 * @param message   message received from stdin
 * @param argc      pointer to int of number of agrs to put values into
 * @param args      pointer to array of char arrys of argv to put values into
 */
void get_newround_args(char* message, int* argc, char*** args) {
    char* buffer = calloc(strlen(message), sizeof(char));

    strcpy(buffer, message);
    memmove(buffer, buffer + strlen("NEWROUND"), strlen(buffer));

    if (strcmp(buffer, "") == 0) {
        quit_on_error(BADMESSAGE);
    }

    for (int i = 0; i < strlen(buffer); i++) {
        if (!isdigit((int) buffer[i])) {
            quit_on_error(BADMESSAGE);
        }
    }

    *argc = 1;

    *args = calloc(*argc, sizeof(char*));
    (*args)[0] = calloc(strlen(buffer), sizeof(char));

    strcpy((*args)[0], buffer);
}

/**
 * gets args from message received for a PLAYED type instruction
 *
 * @param message   message received from stdin
 * @param argc      pointer to int of number of agrs to put values into
 * @param args      pointer to array of char arrys of argv to put values into
 */
void get_played_args(char* message, int* argc, char*** args) {
    char* buffer = calloc(strlen(message), sizeof(char));

    strcpy(buffer, message);
    memmove(buffer, buffer + strlen("PLAYED"), strlen(buffer));

    for (int i = 0; i < strlen(buffer); i++) {
        if (buffer[i] == ',') {
            buffer[i] = 0;
            break;
        }
    }

    if (strcmp(buffer, "") == 0) {
        quit_on_error(BADMESSAGE);
    }

    for (int i = 0; i < strlen(buffer); i++) {
        if (!isdigit((int) buffer[i])) {
            quit_on_error(BADMESSAGE);
        }
    }

    *argc = 2;

    *args = calloc(*argc, sizeof(char*));
    (*args)[0] = calloc(strlen(buffer), sizeof(char));
    (*args)[1] = calloc(2, sizeof(char));

    if (strlen(message) != (9 + strlen(buffer))) {
        quit_on_error(BADMESSAGE);
    }

    strcpy((*args)[0], buffer);
    (*args)[1][0] = message[7 + strlen(buffer)];
    (*args)[1][1] = message[8 + strlen(buffer)];

    if(!valid_card((*args)[1][0], (*args)[1][1])) {
        quit_on_error(BADMESSAGE);
    }
}

/**
 * main function of 2310baseplayer
 *
 * @param argc  number of args received
 * @param argv  values of args received
 * @return      status. 0 if OK
 */
int main(int argc, char** argv) {
    if (argc != 5) {
        quit_on_error(BADARGNUM);
    }

    GameStats gameStats;
    init_player(argv, &gameStats);

    write(STDOUT_FILENO, "@", 1);
    fflush(stdout);

    game_loop(gameStats);

    return OK;
}

/**
 * main loop for game logic
 *
 * @param gameStats stastics for the game
 */
void game_loop(GameStats gameStats) {
    char buffer[BUFFER_SIZE];
    Instruction instruction;
    Card* hand;
    bool gameOver = false;
    get_instruction(&instruction);
    if (strcmp(instruction.type, "HAND") != 0) {
        quit_on_error(BADMESSAGE);
    } else {
        if (instruction.argc != gameStats.handSize) {
            quit_on_error(BADMESSAGE);
        }

        hand = calloc(instruction.argc, sizeof(Card));
        init_hand(instruction.argc, instruction.args, &hand);
    }
    while (!gameOver) {
        // next we want to see a NEWROUND, quit if not
        get_instruction(&instruction);
        if (strcmp(instruction.type, "NEWROUND") != 0) {
            quit_on_error(BADMESSAGE);
        } else {
            if (strtol(instruction.args[0], NULL, 10) >=
                    gameStats.playerCount) {
                quit_on_error(BADMESSAGE);
            }
            gameStats.currentPlayer = strtol(instruction.args[0], NULL, 10)
                    % gameStats.playerCount;
        }

        char* roundHistory = calloc(BUFFER_SIZE, sizeof(char));
        strcpy(roundHistory, "Lead player=");
        sprintf(buffer, "%d", gameStats.currentPlayer);
        strcat(roundHistory, buffer);
        roundHistory[strlen(roundHistory)] = ':';

        char lead = 0;

        for (int i = 0; i < gameStats.playerCount; i++) {
            if (gameStats.currentPlayer == gameStats.position) {
                // i am the captain now
                Card cardPlayed = play_card(lead, gameStats.handSize, hand);

                roundHistory[strlen(roundHistory)] = ' ';
                roundHistory[strlen(roundHistory)] = cardPlayed.suit;
                roundHistory[strlen(roundHistory)] = '.';
                roundHistory[strlen(roundHistory)] = cardPlayed.rank;

                for (int i = 0; i < gameStats.handSize; i++) {
                    if (hand[i].suit == cardPlayed.suit &&
                            hand[i].rank == cardPlayed.rank) {
                        hand[i].suit = 0;
                        hand[i].rank = 0;
                    }
                }

                gameStats.currentPlayer = (gameStats.position + 1)
                        % gameStats.playerCount;
            } else {
                // expecting a PLAYED
                get_instruction(&instruction);
                if (strcmp(instruction.type, "PLAYED") != 0) {
                    quit_on_error(BADMESSAGE);
                }

                if (lead == 0) {
                    lead = instruction.args[1][0];
                }

                roundHistory[strlen(roundHistory)] = ' ';
                roundHistory[strlen(roundHistory)] = instruction.args[1][0];
                roundHistory[strlen(roundHistory)] = '.';
                roundHistory[strlen(roundHistory)] = instruction.args[1][1];

                gameStats.currentPlayer = (strtol(instruction.args[0], NULL, 0)
                        + 1) % gameStats.playerCount;
            }
        }

        roundHistory[strlen(roundHistory)] = '\n';
        fputs(roundHistory, stderr);
        fflush(stderr);
        free(roundHistory);
    }
}

/**
 * end function due to an error, print error to stderr
 *
 * @param s     status of program to exit on
 */
void quit_on_error(Status s) {
    const char* messages[] = {"",
            "Usage: player players myid "
            "threshold handsize\n",
            "Invalid players\n",
            "Invalid position\n",
            "Invalid threshold\n",
            "Invalid hand size\n",
            "Invalid message\n",
            "EOF\n"};
    fputs(messages[s], stderr);
    close(STDIN_FILENO);
    fflush(stdout);
    exit(s);
}