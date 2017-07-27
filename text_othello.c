#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "othello.h"

static void print_game(othello_t *o)
{
        char str[1000];

        othello_to_string(o, str);
        puts(str);
}

static void read_move(int *row, int *col)
{
        char c;

        while (true) {
                if (scanf("%c%d", &c, row) == EOF || getchar() == EOF) {
                        exit(0);
                }
                *col = c - 'a';
                --*row; /* Row 1 is row 0 internally. */

                if (*row < 0 || *row > 7 || *col < 0 || *col > 7) {
                        printf("Invalid input; try again: "); 
                        continue;
                }

                break;
        }
}

static void print_move(int row, int col)
{
        printf("%c%d", "abcdefgh"[col], row + 1);
}

static void play(bool self_play)
{
        othello_t o;
        int i, j, n;

        othello_init(&o);
        n = 0;

        printf("\nNew game!\n");

        while (true) {
                printf("After %d moves:\n", n);
                print_game(&o);

                if (!othello_has_valid_move(&o, PLAYER_BLACK) &&
                    !othello_has_valid_move(&o, PLAYER_WHITE)) {
                        printf("Game over: (black--white) %d--%d\n",
                               othello_score(&o, PLAYER_BLACK),
                               othello_score(&o, PLAYER_WHITE));
                        break;
                }

                if (othello_has_valid_move(&o, PLAYER_BLACK)) {
                        if (self_play) {
                                printf("Black's move.. ");
                                fflush(stdout);
                                if (n < 5) {
                                        othello_compute_random_move(&o,
                                                        PLAYER_BLACK, &i, &j);
                                } else {
                                        othello_compute_move(&o,
                                                        PLAYER_BLACK, &i, &j);
                                }
                                print_move(i, j);
                                printf("\n");
                        } else {
                                printf("Black's move: ");
                                while (true) {
                                        read_move(&i, &j);
                                        if (othello_is_valid_move(&o,
                                                                PLAYER_BLACK,
                                                                i, j)) {
                                                break;
                                        }
                                        printf("Invalid move; try again: ");
                                }
                        }
                        othello_make_move(&o, PLAYER_BLACK, i, j);
                        n++;
                }

                if (othello_has_valid_move(&o, PLAYER_WHITE)) {
                        printf("White's move.. ");
                        fflush(stdout);
                        if (self_play && n < 6) {
                                othello_compute_random_move(&o,
                                                PLAYER_WHITE, &i, &j);
                        } else {
                                othello_compute_move(&o, PLAYER_WHITE, &i, &j);
                        }
                        print_move(i, j);
                        printf("\n");
                        othello_make_move(&o, PLAYER_WHITE, i, j);
                        n++;
                }

                printf("\n");
        }
}

int main(int argc, char **argv)
{
        while (true) {
                play(argc == 2 && !strcmp(argv[1], "self"));
        }

        return 0;
}
