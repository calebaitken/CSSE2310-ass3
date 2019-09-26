// @author Caleb Aitken

/*
 * EXIT CONDITION                           MESSAGE
 * 0    Normal exit
 * 1    Less than 4 command line arguments  Usage: 2310hub deck
 *                                          threshold player0 {player1}
 * 2    Threshold < 2 or not a number       Invalid threshold
 * 3    Problem reading / parsing the deck  Deck error
 * 4    Less than P cards in the deck       Not enough cards
 * 5    Unable to start one of the players  Player error
 * 6    Unexpected EOF from a player        Player EOF
 * 7    Invalid message from a player       Invalid message
 * 8    Player chooses card they don’t have Invalid card choice
 *       or don’t follow suit
 * 9    Received SIGHUP                     Ended due to signal
 */

#include "2310hub.h"

/**
 * Inits the game and checks all args are ok
 *
 * @param playerCount   number of players in the game
 * @param argv          args passed to ./2310hub
 * @param game          game to init
 */
void init_game(int playerCount, char** argv, Game* game) {
    init_threshold(argv[2], &game->threshold);

    init_deck(argv[1], &game->deck);
    if (game->deck.count < playerCount) {
        quit_on_error(BADCARDNUM);
    }

    game->playerCount = playerCount;
    game->numRounds = (int)(game->deck.count / playerCount);
}

/**
 * Inits deck and checks all args are ok
 *
 * @param deckName  deck file to open
 * @param deck      deck to init
 */
void init_deck(const char* deckName, Deck* deck) {
    char buffer[BUFFER_SIZE];
    FILE* deckFile = fopen(deckName, "r");
    if (deckFile == NULL) {
        quit_on_error(DECKERROR);
    }

    int deckSize;
    fgets(buffer, BUFFER_SIZE - 1, deckFile);
    deckSize = strtol(buffer, NULL, 10);
    Card* cards = calloc(deckSize, sizeof(Card));
    for (int i = 0; i < deckSize; i++) {
        strcpy(buffer, "");
        if (!fgets(buffer, BUFFER_SIZE - 1, deckFile)) {
            free(cards);
            quit_on_error(DECKERROR);
        }

        if (!valid_card(buffer[0], buffer[1])) {
            free(cards);
            quit_on_error(DECKERROR);
        }

        cards[i].suit = buffer[0];
        cards[i].rank = buffer[1];
    }

    if(!valid_deck(cards, deckSize)) {
        free(deck);
        quit_on_error(DECKERROR);
    }

    deck->cards = cards;
    deck->count = deckSize;
    deck->used = 0;
    fclose(deckFile);
}

/**
 * inits threshold and validates
 *
 * @param thresholdArg  arg for threshold
 * @param threshold     threshold to init
 */
void init_threshold(const char* thresholdArg, int* threshold) {
    if (!isdigit((int)*thresholdArg)) {
        quit_on_error(BADTHRESHOLD);
    }

    *threshold = strtol(thresholdArg, NULL, 10);
    if (*threshold < 2) {
        quit_on_error(BADTHRESHOLD);
    }
}

/**
 * inits player programs and validates them
 *
 * @param game          game stats
 * @param argv          args sent to ./2310hub
 * @param playerPipes   communication pipes to init
 * @return              array of player PIDs
 */
int* init_players(Game game, char** argv, int*** playerPipes) {
    int* playerPIDs = calloc(game.playerCount, sizeof(int));
    for (int i = 0; i < game.playerCount; i++) {
        char cplayerCount[BUFFER_SIZE];
        char ci[BUFFER_SIZE];
        char cthreshold[BUFFER_SIZE];
        char cnumRounds[BUFFER_SIZE];

        sprintf(cplayerCount, "%d", game.playerCount);
        sprintf(ci, "%d", i);
        sprintf(cthreshold, "%d", game.threshold);
        sprintf(cnumRounds, "%d", game.numRounds);

        char* args[] = {argv[i + 3], cplayerCount, ci,
                cthreshold, cnumRounds, NULL};

        pipe(playerPipes[i][0]);
        pipe(playerPipes[i][1]);

        playerPIDs[i] = create_player_process(args,
                playerPipes[i][1], playerPipes[i][0]);
    }

    if (!verify_players(game.playerCount, playerPipes)) {
        quit_on_error(PLAYERERROR); // last chance to call PLAYERERROR
    }

    return playerPIDs;
}

/**
 * Assigns hands to players and send a message via the pipes to notify players
 *
 * @param deck          deck to pull cards from to give to players
 * @param handSize      size of players' hands
 * @param playerCount   number of players in the game
 * @param playerPipes   communication pipes for players
 */
void assign_hands(Deck deck, int handSize,
        int playerCount, int*** playerPipes) {
    for (int i = 0; i < playerCount; i++) {
        char handBuffer[BUFFER_SIZE];
        char buffer[BUFFER_SIZE];
        memset(&handBuffer, 0, sizeof(handBuffer));
        strcat(handBuffer, "HAND");
        sprintf(buffer, "%d", handSize);
        strcat(handBuffer, buffer);
        for (int j = (0 + (i * handSize)); j < (handSize * (i + 1)); j++) {
            strcat(handBuffer, ",");
            handBuffer[strlen(handBuffer)] = deck.cards[j].suit;
            handBuffer[strlen(handBuffer)] = deck.cards[j].rank;
        }
        strcat(handBuffer, "\n");
        write(playerPipes[i][1][1], handBuffer, strlen(handBuffer));
    }
}

/**
 * Send a NEWROUND message to all players in the game
 *
 * @param leadPlayer    the player leading this round
 * @param playerCount   number of players in the game
 * @param playerPipes   communication pipes for players
 */
void start_round(int leadPlayer, int playerCount, int*** playerPipes) {
    char buffer[BUFFER_SIZE];
    char cleadPlayer[BUFFER_SIZE];

    memset(&buffer, 0, sizeof(buffer));
    memset(&cleadPlayer, 0, sizeof(cleadPlayer));

    strcpy(buffer, "NEWROUND");
    sprintf(cleadPlayer, "%d", leadPlayer);
    strcat(buffer, cleadPlayer);
    strcat(buffer, "\n");

    fprintf(stdout, "Lead player=%d\n", leadPlayer);
    fflush(stdout);

    for (int i = 0; i < playerCount; i++) {
        write(playerPipes[i][1][1], buffer, strlen(buffer));
    }
}

/**
 * Gets the card a player wants to play from their pipe
 *
 * @param currentPlayer the player to get a card from
 * @param playerPipes   communication pipes for the players
 * @return              card play selected from their hand
 */
Card get_play(int currentPlayer, int*** playerPipes) {
    char message[BUFFER_SIZE];
    size_t bytesRead = 0;

    memset(&message, 0, sizeof(message));

    // get message from player
    bytesRead = read(playerPipes[currentPlayer][0][0],
            message, sizeof(message));
    if (bytesRead == 0) {
        quit_on_error(PLAYEREOF);
    }

    // verify length
    if (strlen(message) != 7) {
        quit_on_error(BADMSG);
    }

    // verify that the message is PLAY
    char* buffer = calloc(strlen(message), sizeof(char));
    strcpy(buffer, message);
    buffer[strlen("PLAY")] = 0;
    buffer[strlen(message)] = 0;
    if (strcmp(buffer, "PLAY") != 0) {
        quit_on_error(BADMSG);
    }

    // get card played
    Card cardPlayed;
    cardPlayed.suit = message[4];
    cardPlayed.rank = message[5];

    // validate card
    if (!valid_card(cardPlayed.suit, cardPlayed.rank)) {
        quit_on_error(BADMSG);
    }

    return cardPlayed;
}

/**
 * Prints the move made by a player to the stdout
 *
 * @param currentPlayer play that played a card
 * @param card          card that the palyer played
 * @param playerCount   number of players in the game
 * @param playerPipes   communication pipes for players
 */
void print_move(int currentPlayer, Card card,
        int playerCount, int*** playerPipes) {
    char message[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];

    memset(&message, 0, sizeof(message));
    memset(&buffer, 0, sizeof(buffer));

    strcpy(message, "PLAYED");
    sprintf(buffer, "%d", currentPlayer);
    strcat(message, buffer);
    strcat(message, ",");
    message[strlen(message)] = card.suit;
    message[strlen(message)] = card.rank;
    strcat(message, "\n");

    for (int i = 0; i < playerCount; i++) {
        if (i != currentPlayer) {
            write(playerPipes[i][1][1], message, strlen(message));
        }
    }
}

/**
 * Calculates which player won the round
 *
 * @param playerCount   number of players in the game
 * @param cardsPlayed   list of cards played with round
 * @return              position of player that won
 */
int find_winner(int playerCount, Card* cardsPlayed) {
    int highestPlayer = 0;
    Card highestCard = cardsPlayed[0];

    for (int i = 1; i < playerCount; i++) {
        if ((int)cardsPlayed[i].suit == (int)highestCard.suit &&
                (int)cardsPlayed[i].rank > (int)highestCard.rank) {
            highestPlayer = i;
            highestCard = cardsPlayed[i];
        }
    }

    return highestPlayer;
}

/**
 * Counts the number of d cards played this round
 *
 * @param playerCount   number of players in the game
 * @param cardsPlayed   list of cards played with round
 * @return              number of d cards played
 */
int count_d_cards(int playerCount, Card* cardsPlayed) {
    int count = 0;
    for (int i = 0; i < playerCount; i++) {
        if ((int)cardsPlayed[i].suit == (int)'D') {
            count++;
        }
    }

    return count;
}

/**
 * calculates and prints the scores of the players at the end of the game
 *
 * @param playerCount   number of players in the the game
 * @param threshold     number of d cards that must be played for them
 *                      to be counted as positive
 * @param scores        list of how many rounds each player has won
 * @param dCards        list of how many dcards each player has had in a round
 *                      they've won
 */
void print_scores(int playerCount, int threshold, int* scores, int* dCards) {
    char buffer[BUFFER_SIZE];
    char intBuffer[BUFFER_SIZE];

    memset(&buffer, 0, sizeof(buffer));
    memset(intBuffer, 0, sizeof(intBuffer));

    for (int i = 0; i < playerCount; i++) {
        int finalScore;
        if (dCards[i] < threshold) {
            finalScore = scores[i] - dCards[i];
        } else {
            finalScore = scores[i] + dCards[i];
        }
        sprintf(intBuffer, "%d", i);
        strcat(buffer, intBuffer);
        strcat(buffer, ":");
        sprintf(intBuffer, "%d", finalScore);
        strcat(buffer, intBuffer);
        strcat(buffer, " ");
    }
    buffer[strlen(buffer) - 1] = '\n';
    fputs(buffer, stdout);
    fflush(stdout);
}

/**
 * Sends a GAMEOVER message to all players in the game
 *
 * @param playerCount   number of players in the game
 * @param playerPipes   communication pipes for players
 */
void gameover(int playerCount, int*** playerPipes) {
    char message[BUFFER_SIZE];

    memset(&message, 0, sizeof(message));

    strcpy(message, "GAMEOVER\n");

    for (int i = 0; i < playerCount; i++) {
        write(playerPipes[i][1][1], message, strlen(message));
    }
}

/**
 * main function of ./2310hub
 *
 * @param argc  number of args
 * @param argv  values of args
 * @return      status. 0 if OK
 */
int main(int argc, char** argv) {
    signal(SIGHUP, handle_sighup);
    if (argc < 5) {
        quit_on_error(BADARGNUM);
    }

    Game game;
    game.playerCount = argc - 3;
    init_game(game.playerCount, argv, &game);

    int*** playerPipes;
    playerPipes = calloc(game.playerCount, sizeof(int**));
    for (int i = 0; i < game.playerCount; i++) {
        playerPipes[i] = calloc(2, sizeof(int*));
        for (int j = 0; j < 2; j++) {
            playerPipes[i][j] = calloc(2, sizeof(int));
        }
    }

    init_players(game, argv, playerPipes);

    assign_hands(game.deck, game.numRounds, game.playerCount, playerPipes);

    game_loop(game, playerPipes);

    gameover(game.playerCount, playerPipes);
    return OK;
}

/**
 * main loop for the game logic. prints out scores on finish
 *
 * @param game          stats of the game
 * @param playerPipes   communication pipes for player
 */
void game_loop(Game game, int*** playerPipes) {
    int leadPlayer = 0;
    int currentPlayer;
    int* scores = calloc(game.numRounds, sizeof(int));
    int* dCards = calloc(game.numRounds, sizeof(int));
    for (int i = 0; i < game.numRounds; i++) {
        start_round(leadPlayer, game.playerCount, playerPipes);
        currentPlayer = leadPlayer;
        Card* cardsPlayed = calloc(game.playerCount, sizeof(Card));
        char cardsBuffer[BUFFER_SIZE];
        memset(&cardsBuffer, 0, sizeof(cardsBuffer));
        strcpy(cardsBuffer, "Cards=");
        for (int j = 0; j < game.playerCount; j++) {
            Card card = get_play(((currentPlayer + j) % game.playerCount),
                    playerPipes);
            print_move(((currentPlayer + j) % game.playerCount), card,
                    game.playerCount, playerPipes);
            cardsPlayed[j] = card;
            cardsBuffer[strlen(cardsBuffer)] = card.suit;
            strcat(cardsBuffer, ".");
            cardsBuffer[strlen(cardsBuffer)] = card.rank;
            strcat(cardsBuffer, " ");
        }
        cardsBuffer[strlen(cardsBuffer) - 1] = '\n';
        fputs(cardsBuffer, stdout);
        fflush(stdout);
        leadPlayer = (find_winner(game.playerCount, cardsPlayed) +
                leadPlayer) % game.playerCount;
        scores[leadPlayer] += 1;
        dCards[leadPlayer] += count_d_cards(game.playerCount, cardsPlayed);
    }

    print_scores(game.playerCount, game.threshold, scores, dCards);
}

/**
 * forks and executes a player process
 *
 * @param args          args to execute player with
 * @param childRead     pipes that player will read from
 * @param childWrite    pipes that palyer will write to
 * @return              PID of child function
 */
int create_player_process(char* args[], int childRead[2], int childWrite[2]) {
    int pid = fork();
    if (pid == 0) {
        // this is the child
        /*
        fputs(args[0], stdout);
        fflush(stdout);
        fputs(" has started\n", stdout);
        fflush(stdout);
         */

        dup2(childRead[0], STDIN_FILENO);
        close(childRead[1]);
        close(childRead[0]);
        dup2(childWrite[1], STDOUT_FILENO);
        close(childWrite[0]);
        close(childWrite[1]);

        close(STDERR_FILENO);

        execvp(args[0], args);

        fputs("exec has failed\n", stdout);
        fflush(stdout);
        quit_on_error(PLAYERERROR);
    } else if (pid == -1) {
        // fork failed
        quit_on_error(PLAYERERROR);
    }
    return pid;
}

/**
 * verifies that players have started up correctly
 *
 * @param playerCount   number of players in the game
 * @param playerPipes   communication pipes of players
 * @return              true if a player does not send '@', true otherwise
 */
bool verify_players(int playerCount, int*** playerPipes) {
    char buffer[BUFFER_SIZE];
    for (int i = 0; i < playerCount; i++) {
        strcpy(buffer, "");
        size_t bytesRead = 0;

        bytesRead = read(playerPipes[i][0][0], buffer, 1);
        if (bytesRead == 0) {
            return false;
        }

        if (strcmp(buffer, "@") != 0) {
            return false;
        }
    }

    return true;
}

/**
 * handle SIGHUP signal
 *
 * @param sig   signal id
 */
void handle_sighup(int sig) {
    quit_on_error(SSIGHUP);
}

/**
 * end function due to an error, print error to stderr
 *
 * @param s     status of program to exit on
 */
void quit_on_error(Status s) {
    // compound string literal for status messages
    const char* statusMessages[] = {"",
            "Usage: 2310hub deck threshold "
            "player0 {player1}\n",
            "Invalid threshold\n",
            "Deck error\n",
            "Not enough cards\n",
            "Player error\n",
            "Player EOF\n",
            "Invalid message\n",
            "Invalid card choice\n",
            "Ended due to signal\n"};
    fputs(statusMessages[s], stderr);
    fflush(stderr);
    exit(s);
}