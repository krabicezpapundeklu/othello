#import <Cocoa/Cocoa.h>
#include "../othello.h"

enum State { BLACKS_MOVE, WHITES_MOVE, GAME_OVER };

@interface AppDelegate : NSObject <NSApplicationDelegate> {
    othello_t board;
    enum State state;
}

// Get a pointer to the Othello board.
- (const othello_t*)board;

// Get the current state.
- (enum State)state;

// To be called when a cell on the board is clicked by the user.
- (void)boardWasClickedAtRow:(int)row Column:(int)col;

// Reset the game state.
- (IBAction)newGame:(id)sender;

@end
