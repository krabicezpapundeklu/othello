#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "othello.h"

static void check_moves(const char *board_str, player_t p,
                        const char *expected_moves)
{
        othello_t o, m;
        int row, col;
        bool v;
        char s[1000];

        othello_from_string(board_str, &o);

        /* Put a black disk in each cell of m that is a valid move for p. */
        othello_init(&m);
        for (row = 0; row < 8; row++) {
                for (col = 0; col < 8; col++) {
                        v = othello_is_valid_move(&o, p, row, col);
                        othello_set_cell_state(&m, row, col,
                                               v ? CELL_BLACK : CELL_EMPTY);
                }
        }

        othello_to_string(&m, s);

        if (strcmp(expected_moves, s) != 0) {
                fprintf(stderr, "Error! Expected moves:\n%sbut got:\n%s",
                                expected_moves, s);
                exit(EXIT_FAILURE);
        }
}

static void check_resolve(const char *before, const char *after, player_t p,
                          int row, int col)
{
        othello_t o;
        char s[1000];

        othello_from_string(before, &o);
        othello_make_move(&o, p, row, col);
        othello_to_string(&o, s);

        if (strcmp(after, s) != 0) {
                fprintf(stderr, "Error! Expected result:\n%sbut got:\n%s",
                                after, s);
                exit(EXIT_FAILURE);
        }
}

static void test_gen_moves_init(void)
{
        /* Test move generation from the initial position. */

        const char board[] =
        " abcdefgh \n"
        "1........1\n"
        "2........2\n"
        "3........3\n"
        "4...ox...4\n"
        "5...xo...5\n"
        "6........6\n"
        "7........7\n"
        "8........8\n"
        " abcdefgh \n";

        const char black_moves[] =
        " abcdefgh \n"
        "1........1\n"
        "2........2\n"
        "3...x....3\n"
        "4..x.....4\n"
        "5.....x..5\n"
        "6....x...6\n"
        "7........7\n"
        "8........8\n"
        " abcdefgh \n";

        check_moves(board, PLAYER_BLACK, black_moves);
}

static void test_gen_moves_7_steps(void)
{
        /* Test that we can find a move a full 7 steps away. */

        const char board[] =
        " abcdefgh \n"
        "1........1\n"
        "2......o.2\n"
        "3.....o..3\n"
        "4....o...4\n"
        "5...o....5\n"
        "6..o.....6\n"
        "7.o......7\n"
        "8x.......8\n"
        " abcdefgh \n";

        const char moves[] =
        " abcdefgh \n"
        "1.......x1\n"
        "2........2\n"
        "3........3\n"
        "4........4\n"
        "5........5\n"
        "6........6\n"
        "7........7\n"
        "8........8\n"
        " abcdefgh \n";

        check_moves(board, PLAYER_BLACK, moves);
}

static void test_gen_moves_all_dirs(void)
{
        /* Test that we find moves in all directions. */

        const char board[] =
        " abcdefgh \n"
        "1........1\n"
        "2........2\n"
        "3...ooo..3\n"
        "4...oxo..4\n"
        "5...ooo..5\n"
        "6........6\n"
        "7........7\n"
        "8........8\n"
        " abcdefgh \n";

        const char moves[] =
        " abcdefgh \n"
        "1........1\n"
        "2..x.x.x.2\n"
        "3........3\n"
        "4..x...x.4\n"
        "5........5\n"
        "6..x.x.x.6\n"
        "7........7\n"
        "8........8\n"
        " abcdefgh \n";

        check_moves(board, PLAYER_BLACK, moves);
}

static void test_gen_moves_no_wrap_l(void)
{
        /* Test that we don't wrap around to the left. */

        const char board[] =
        " abcdefgh \n"
        "1........1\n"
        "2........2\n"
        "3o.......3\n"
        "4ox......4\n"
        "5o.......5\n"
        "6........6\n"
        "7........7\n"
        "8........8\n"
        " abcdefgh \n";

        const char moves[] =
        " abcdefgh \n"
        "1........1\n"
        "2........2\n"
        "3........3\n"
        "4........4\n"
        "5........5\n"
        "6........6\n"
        "7........7\n"
        "8........8\n"
        " abcdefgh \n";

        check_moves(board, PLAYER_BLACK, moves);
}

static void test_gen_moves_no_wrap_r(void)
{
        /* Test that we don't wrap around to the right. */

        const char board[] =
        " abcdefgh \n"
        "1........1\n"
        "2........2\n"
        "3.......o3\n"
        "4......xo4\n"
        "5.......o5\n"
        "6........6\n"
        "7........7\n"
        "8........8\n"
        " abcdefgh \n";

        const char moves[] =
        " abcdefgh \n"
        "1........1\n"
        "2........2\n"
        "3........3\n"
        "4........4\n"
        "5........5\n"
        "6........6\n"
        "7........7\n"
        "8........8\n"
        " abcdefgh \n";

        check_moves(board, PLAYER_BLACK, moves);
}

static void test_resolve_7_steps(void)
{
        /* Test resolving 7 steps away from our disk. */

        const char before[] =
        " abcdefgh \n"
        "1........1\n"
        "2......o.2\n"
        "3.....o..3\n"
        "4....o...4\n"
        "5...o....5\n"
        "6..o.....6\n"
        "7.o......7\n"
        "8x.......8\n"
        " abcdefgh \n";

        const char after[] =
        " abcdefgh \n"
        "1.......x1\n"
        "2......x.2\n"
        "3.....x..3\n"
        "4....x...4\n"
        "5...x....5\n"
        "6..x.....6\n"
        "7.x......7\n"
        "8x.......8\n"
        " abcdefgh \n";

        check_resolve(before, after, PLAYER_BLACK, 0, 7);
}

static void test_resolve_all_dirs(void)
{
        /* Test resolving in all directions. */

        const char before[] =
        " abcdefgh \n"
        "1........1\n"
        "2..x.x.x.2\n"
        "3...ooo..3\n"
        "4..xo.ox.4\n"
        "5...ooo..5\n"
        "6..x.x.x.6\n"
        "7........7\n"
        "8........8\n"
        " abcdefgh \n";

        const char after[] =
        " abcdefgh \n"
        "1........1\n"
        "2..x.x.x.2\n"
        "3...xxx..3\n"
        "4..xxxxx.4\n"
        "5...xxx..5\n"
        "6..x.x.x.6\n"
        "7........7\n"
        "8........8\n"
        " abcdefgh \n";

        check_resolve(before, after, PLAYER_BLACK, 3, 4);
}

static void test_resolve_no_wrap_l(void)
{
        /* Test that we don't wrap around to the left. */

        const char before[] =
        " abcdefgh \n"
        "1.......x1\n"
        "2.......x2\n"
        "3o......x3\n"
        "4o.ox...x4\n"
        "5o......x5\n"
        "6.......x6\n"
        "7.......x7\n"
        "8.......x8\n"
        " abcdefgh \n";

        const char after[] =
        " abcdefgh \n"
        "1.......x1\n"
        "2.......x2\n"
        "3o......x3\n"
        "4oxxx...x4\n"
        "5o......x5\n"
        "6.......x6\n"
        "7.......x7\n"
        "8.......x8\n"
        " abcdefgh \n";

        check_resolve(before, after, PLAYER_BLACK, 3, 1);
}

static void test_resolve_no_wrap_r(void)
{
        /* Test that we don't wrap around to the right. */

        const char before[] =
        " abcdefgh \n"
        "1x.......1\n"
        "2x.......2\n"
        "3x......o3\n"
        "4x...xo.o4\n"
        "5x......o5\n"
        "6x.......6\n"
        "7x.......7\n"
        "8x.......8\n"
        " abcdefgh \n";

        const char after[] =
        " abcdefgh \n"
        "1x.......1\n"
        "2x.......2\n"
        "3x......o3\n"
        "4x...xxxo4\n"
        "5x......o5\n"
        "6x.......6\n"
        "7x.......7\n"
        "8x.......8\n"
        " abcdefgh \n";

        check_resolve(before, after, PLAYER_BLACK, 3, 6);
}

static void test_winning_move(void)
{
        /* A basic test that we can compute a winning move. */

        othello_t o;
        int row, col;

        const char board[] =
        " abcdefgh \n"
        "1.....x.o1\n"
        "2oxxxx.oo2\n"
        "3oxxxxooo3\n"
        "4oxoxoxoo4\n"
        "5ooxoxxoo5\n"
        "6oooxoooo6\n"
        "7ooooxoox7\n"
        "8oxxxxxxx8\n"
        " abcdefgh \n";

        othello_from_string(board, &o);

        othello_compute_move(&o, PLAYER_WHITE, &row, &col);

        if (row != 0 && col != 0) {
                fprintf(stderr, "expected A1 but got %c%d\n",
                                "ABCDEFGH"[col], row + 1);
                exit(EXIT_FAILURE);
        }
}

static const struct {
        const char *name;
        void (*f)(void);
} tests[] = {
        { "gen_moves_init",      test_gen_moves_init },
        { "gen_moves_7_steps",   test_gen_moves_7_steps },
        { "gen_moves_all_dirs",  test_gen_moves_all_dirs },
        { "gen_moves_no_wrap_l", test_gen_moves_no_wrap_l },
        { "gen_moves_no_wrap_r", test_gen_moves_no_wrap_r },
        { "resolve_7_steps",     test_resolve_7_steps },
        { "resolve_all_dirs",    test_resolve_all_dirs },
        { "resolve_no_wrap_l",   test_resolve_no_wrap_l },
        { "resolve_no_wrap_r",   test_resolve_no_wrap_r },
        { "winning_move",        test_winning_move }
};

int main()
{
        size_t i;

        for (i = 0; i < sizeof(tests) / sizeof(tests[0]); i++) {
                printf("%-20s", tests[i].name);
                fflush(stdout);
                (tests[i].f)();
                printf("OK\n");
        }

        return 0;
}
