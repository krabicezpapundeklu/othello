#include <assert.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include "othello.h"

#define BENCH_TIME 5  /* In seconds. */

static volatile bool running;

static void alarm_handler(int sig)
{
        assert(sig == SIGALRM);

        running = false;

        if (signal(SIGALRM, SIG_DFL) == SIG_ERR) {
                perror("signal");
                exit(1);
        }
}

static void set_alarm(unsigned secs)
{
        if (signal(SIGALRM, alarm_handler) == SIG_ERR) {
                perror("signal");
                exit(1);
        }

        alarm(secs);
}

static double get_time(void)
{
        struct timeval tv;

        if (gettimeofday(&tv, NULL) != 0) {
                perror("gettimeofday");
                exit(1);
        }

        return (double)tv.tv_sec + (double)tv.tv_usec / 1e6;
}

static othello_t test_board;
static othello_t scratch_board;

static void bench_gen_moves(void)
{
        othello_has_valid_move(&test_board, PLAYER_BLACK);
}

static void bench_resolve_move(void)
{
        memcpy(&scratch_board, &test_board, sizeof(scratch_board));
        othello_make_move(&scratch_board, PLAYER_BLACK, 5, 4);
}

static void bench_eval(void)
{
        othello_eval(&test_board, PLAYER_BLACK);
}

static void bench_negamax(void)
{
        othello_negamax(&test_board, PLAYER_BLACK, 5);
}

static void bench_iter_negamax(void)
{
        othello_iterative_negamax(&test_board, PLAYER_BLACK, 50000);
}

static const struct {
        const char *name;
        void (*f)(void);
} benchmarks[] = {
        { "gen_moves",    bench_gen_moves },
        { "resolve_move", bench_resolve_move },
        { "eval",         bench_eval },
        { "negamax5",     bench_negamax },
        { "iter_negamax", bench_iter_negamax },
};

static void run_benchmark(int benchmark_idx)
{
        double start, stop;
        uint64_t iterations;

        running = true;
        iterations = 0;
        set_alarm(BENCH_TIME);
        start = get_time();

        while (running) {
                (benchmarks[benchmark_idx].f)();
                ++iterations;
        }

        stop = get_time();

        printf("%12.0f /s\n", iterations / (stop - start));
}

int main()
{
        size_t i;

        othello_init(&test_board);

        for (i = 0; i < sizeof(benchmarks) / sizeof(benchmarks[0]); i++) {
                printf("%-20s", benchmarks[i].name);
                fflush(stdout);
                run_benchmark(i);
        }

        return 0;
}
