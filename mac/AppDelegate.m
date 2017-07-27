#import "AppDelegate.h"
#import "OthelloView.h"

@interface AppDelegate ()
- (void)makeMoveAtRow:(int)row Column:(int)col;

@property (weak) IBOutlet NSWindow *window;
@property (weak) IBOutlet OthelloView *othelloView;
@property (weak) IBOutlet NSMenuItem *newgameMenu;
@end

@implementation AppDelegate

- (id)init {
    self = [super init];
    if (self) {
        [self newGame:nil];
    }
    return self;
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)app {
    (void)app;
    return YES;
}

- (const othello_t*)board {
    return &board;
}

- (enum State)state {
    return state;
}

- (void)boardWasClickedAtRow:(int)row Column:(int)col {
    if (state == BLACKS_MOVE) {
        [self makeMoveAtRow:row Column:col];
    }
}

- (IBAction)newGame:(id)sender {
    (void)sender;
    othello_init(&board);
    state = BLACKS_MOVE;
    [self.newgameMenu setEnabled:YES];
    [self.othelloView setNeedsDisplay:YES];
}

- (void)makeMoveAtRow:(int)row Column:(int)col {
    assert(state == BLACKS_MOVE || state == WHITES_MOVE);

    if (state == BLACKS_MOVE) {
        if (!othello_is_valid_move(&board, PLAYER_BLACK, row, col)) {
            // Illegal move; ignored.
            return;
        }

        othello_make_move(&board, PLAYER_BLACK, row, col);
        state = WHITES_MOVE;
    } else {
        othello_make_move(&board, PLAYER_WHITE, row, col);
        state = BLACKS_MOVE;
    }

    if (!othello_has_valid_move(&board, PLAYER_BLACK) &&
        !othello_has_valid_move(&board, PLAYER_WHITE)) {
        state = GAME_OVER;
    } else if (state == BLACKS_MOVE &&
               !othello_has_valid_move(&board, PLAYER_BLACK)) {
        state = WHITES_MOVE;
    } else if (state == WHITES_MOVE &&
               !othello_has_valid_move(&board, PLAYER_WHITE)) {
        state = BLACKS_MOVE;
    }

    if (state == WHITES_MOVE) {
        dispatch_async(dispatch_get_global_queue(
                    DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
            int row, col;
            othello_compute_move(&board, PLAYER_WHITE, &row, &col);

            dispatch_async(dispatch_get_main_queue(), ^{
                [self makeMoveAtRow:row Column:col];
            });
        });
    }

    [self.newgameMenu setEnabled:(state != WHITES_MOVE)];
    [self.othelloView setNeedsDisplay:YES];
}

@end
