//
// Created by caleb on 2019-09-24.
//

#include "2310baseplayer.h"

/**
 * Picks a card to play based on alice's logic:
 *      1. If we are the lead player:
 *          Check each of the suits in this order S, C, D, H. If we have at
 *          least one card in the suit, play the highest ranked one.
 *      2. If we have a card in the lead suit:
 *          Play the lowest card in the lead suit
 *      3. Check the suits in the following order D, H, S, C
 *          If we have a card for that suit, play the highest one.
 *
 * @param lead  the suit of the lead card played
 * @param count the number of cards in hand
 * @param hand  list of cards that can be played
 * @return      the card played
 */
Card play_card(char lead, int count, Card* hand) {
    Card cardToPlay;
    cardToPlay.suit = 0;

    if ((int)lead == 0) {
        cardToPlay.rank = 0;
        const char suitOrder[] = {'S', 'C', 'D', 'H'};

        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < count; j++) {
                if ((int) hand[j].suit == (int) suitOrder[i]) {
                    if ((int) cardToPlay.suit == 0) {
                        cardToPlay = hand[j];
                    } else if ((int) hand[j].rank > (int) cardToPlay.rank) {
                        cardToPlay = hand[j];
                    }
                }
            }
        }
    } else {
        cardToPlay.rank = 127;
        for (int i = 0; i < count; i++) {
            if ((int) hand[i].suit == (int) lead) {
                if ((int) cardToPlay.suit == 0) {
                    cardToPlay = hand[i];
                } else if ((int) hand[i].rank < (int) cardToPlay.rank) {
                    cardToPlay = hand[i];
                }
            }
        }
    }

    if ((int) cardToPlay.suit == 0) {
        const char suitOrder[] = {'D', 'H', 'S', 'C'};
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < count; j++) {
                if ((int) (hand[j].suit) == (int) suitOrder[i]) {
                    if ((int) cardToPlay.suit == 0) {
                        cardToPlay = hand[j];
                    } else if ((int) hand[j].rank > (int) cardToPlay.rank) {
                        cardToPlay = hand[j];
                    }
                }
            }

            if (cardToPlay.suit != 0) {
                break;
            }
        }
    }

    char buffer[7];
    strcpy(buffer, "PLAY");
    buffer[4] = cardToPlay.suit;
    buffer[5] = cardToPlay.rank;
    strcat(buffer, "\n");

    write(STDOUT_FILENO, buffer, strlen(buffer));
    fflush(stdout);

    return cardToPlay;
}