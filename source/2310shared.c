//
// Created by caleb on 2019-09-23.
//

#include "2310shared.h"

/**
 * verfies that a card is legal
 *
 * @param suit  suit of card
 * @param rank  rank of card
 * @return      true if legal, false otherwise
 */
bool valid_card(char suit, char rank) {
    switch ((int)suit) {
        case 83:    // S
            break;
        case 67:    // C
            break;
        case 68:    // D
            break;
        case 72:    // H
            break;
        default:
            return false;
    }

    if (isdigit((int)rank)) {
        return true;
    }

    switch ((int)rank) {
        case 97:    // a
            break;
        case 98:    // b
            break;
        case 99:    // c
            break;
        case 100:   // d
            break;
        case 101:   // e
            break;
        case 102:   // f
            break;
        default:
            return false;
    }

    return true;
}

/**
 * verifies if a deck is legal
 *
 * @param deck  list of cards in deck
 * @param size  number of cards in deck
 * @return      true if legal, false otherwise
 */
bool valid_deck(Card* deck, int size) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (deck[i].rank == deck[j].rank &&
                    deck[i].suit == deck[j].suit && i != j) {
                return false;
            }
        }
    }

    return true;
}

/**
 * verifies if the number of players is legal
 *
 * @param playerCount   number of players in the game
 * @return              true if legal, false otherwise
 */
bool valid_player_count(const char* playerCount) {
    for (int i = 0; i < strlen(playerCount); i++) {
        if (!isdigit((int)playerCount[i])) {
            return false;
        }
    }

    if (strtol(playerCount, NULL, 10) < 2) {
        return false;
    }

    return true;
}

/**
 * verifies if d card threshold is legal
 *
 * @param threshold     threshold
 * @return              true if legal, false otherwise
 */
bool valid_threshold(const char* threshold) {
    for (int i = 0; i < strlen(threshold); i++) {
        if (!isdigit((int)threshold[i])) {
            return false;
        }
    }

    if (strtol(threshold, NULL, 10) < 2) {
        return false;
    }

    return true;
}

/**
 * verifies if hand size is legal
 *
 * @param handSize  number of cards to be put in hand
 * @return          true if legal, false otherwise
 */
bool valid_hand_size(const char* handSize) {
    for (int i = 0; i < strlen(handSize); i++) {
        if (!isdigit((int)handSize[i])) {
            return false;
        }
    }

    if (strtol(handSize, NULL, 10) < 1) {
        return false;
    }

    return true;
}

/**
 * verifies if the player's position is legal
 *
 * @param position      position of the player
 * @param playerCount   number of players in te game
 * @return              true if legal, false otherwise
 */
bool valid_position(const char* position, int playerCount) {
    for (int i = 0; i < strlen(position); i++) {
        if (!isdigit((int)position[i])) {
            return false;
        }
    }

    if (strtol(position, NULL, 10) >= playerCount) {
        return false;
    }

    return true;
}