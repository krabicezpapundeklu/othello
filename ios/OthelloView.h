#import <UIKit/UIKit.h>
#import "OthelloViewController.h"
#include "../othello.h"

@protocol OthelloViewDelegate <NSObject>
- (void)boardWasTappedAtRow:(int)row col:(int)col;
- (void)newGame;
@end

@interface OthelloView : UIView {
    const othello_t *board;
    enum State state;
    __weak id<OthelloViewDelegate> delegate;
    int cellSize, xOffset, yOffset, size;
}

- (id)initWithBoard:(const othello_t*)b state:(enum State)s
           delegate:(id<OthelloViewDelegate>)d;

- (void)updateWithState:(enum State)state;

@end
