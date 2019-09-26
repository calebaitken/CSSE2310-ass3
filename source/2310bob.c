//
// Created by caleb on 2019-09-12.
//

#include "2310baseplayer.h"

/**
 * Picks a card to play based on alice's logic:
 *  1.  If we are the lead player:
 *          Check each of the suits in this order D, H, S, C. If we have at
 *          least one card in the suit, play the lowest ranked one
 *  2.  If at least one other player (including us) has won at least
 *      (threshold−2) D cards and some D cards have been played this round:
 *          If we have a card in the lead suit
 *              Play the highest card in the lead suit
 *          Check each of the suits in this order S, C, H, D
 *              If you have a card of that suit, play the lowest one
 *  3.  If we have a card in the lead suit:
 *          Play the lowest card in the lead suit
 *  4. Check the suits in this order S, C, D, H
 *          If you have at least one card in the suit, play the highest.
 *
 * @param lead  the suit of the lead card played
 * @param count the number of cards in hand
 * @param hand  list of cards that can be played
 * @return      the card played
 */
Card play_card(char lead, int count, Card* hand) {
    Card cardToPlay;
    cardToPlay.suit = 0;
    if (lead == 0) {
        cardToPlay.rank = 127;
        const char suitOrder[] = {'D', 'H', 'S', 'C'};
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < count; j++) {
                if ((int)(hand[j].suit) == (int)suitOrder[i]) {
                    if (cardToPlay.suit != 0) {
                        if ((int)hand[j].rank < (int)cardToPlay.rank) {
                            cardToPlay = hand[j];
                        }
                    } else {
                        cardToPlay = hand[j];
                    }
                }
            }

            if (cardToPlay.suit != 0) {
                break;
            }
        }
    } else {
        cardToPlay.rank = 0;
        const char suitOrder[] = {'S', 'C', 'D', 'H'};
        for (int i = 0; i < count; i++) {
            if ((int)hand[i].suit == (int)lead) {
                if (cardToPlay.suit != 0) {
                    if ((int)hand[i].rank < (int)cardToPlay.rank) {
                        cardToPlay = hand[i];
                    }
                } else {
                    cardToPlay = hand[i];
                }
            }
        }

        if (cardToPlay.suit == 0) {
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < count; j++) {
                    if ((int)(hand[j].suit) == (int)suitOrder[i]) {
                        if (cardToPlay.suit != 0) {
                            if ((int)hand[j].rank > (int)cardToPlay.rank) {
                                cardToPlay = hand[j];
                            }
                        } else {
                            cardToPlay = hand[j];
                        }
                    }
                }

                if (cardToPlay.suit != 0) {
                    break;
                }
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