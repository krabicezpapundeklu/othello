#import "OthelloViewController.h"
#import "OthelloView.h"

@interface OthelloViewController () <OthelloViewDelegate>
- (void)makeMoveAtRow:(int)row Column:(int)col;
- (void)computeWhiteMove;
@end

@implementation OthelloViewController

- (id)init {
    self = [super init];
    if (self) {
        [self newGame];
        self.restorationClass = self.class;
        self.restorationIdentifier = NSStringFromClass(self.class);
    }
    return self;
}

- (void)encodeRestorableStateWithCoder:(NSCoder*)coder {
    uint8_t boardBytes[64];
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            boardBytes[row * 8 + col] = othello_cell_state(&board, row, col);
        }
    }
    [coder encodeBytes:boardBytes length:sizeof(boardBytes) forKey:@"board"];
    [coder encodeInt:(int)state forKey:@"state"];

    [super encodeRestorableStateWithCoder:coder];
}

+ (UIViewController*)
viewControllerWithRestorationIdentifierPath:(NSArray*)identifierComponents
                                      coder:(NSCoder*)coder {
    OthelloViewController *ovc = [[self alloc] init];
    [UIApplication sharedApplication].delegate.window.rootViewController = ovc;

    return ovc;
}

- (void)decodeRestorableStateWithCoder:(NSCoder*)coder {
    NSUInteger length;
    const uint8_t *boardBytes = [coder decodeBytesForKey:@"board"
            returnedLength:&length];
    if (length != 64) {
        return;
    }
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            othello_set_cell_state(&board, row, col, boardBytes[row * 8 + col]);
        }
    }

    state = [coder decodeIntForKey:@"state"];

    [super decodeRestorableStateWithCoder:coder];

    if (state == WHITES_MOVE) {
        [self computeWhiteMove];
    }
}

- (void)loadView {
    self.view = [[OthelloView alloc] initWithBoard:&board state:state
        delegate:self];
}

- (void)boardWasTappedAtRow:(int)row col:(int)col {
    if (state == BLACKS_MOVE) {
        [self makeMoveAtRow:row Column:col];
    }
}

- (void)newGame {
    state = BLACKS_MOVE;
    othello_init(&board);
    [(OthelloView*)self.view updateWithState:state];
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
        [self computeWhiteMove];
    }

    [(OthelloView *)self.view updateWithState:state];
}

- (void)computeWhiteMove {
    assert(state == WHITES_MOVE);

    dispatch_async(
        dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
        ^(void) {
          int row, col;
          othello_compute_move(&board, PLAYER_WHITE, &row, &col);

          dispatch_async(dispatch_get_main_queue(), ^(void) {
            [self makeMoveAtRow:row Column:col];
          });
        });
}

@end
