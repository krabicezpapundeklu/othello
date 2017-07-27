#import <UIKit/UIKit.h>
#include "../othello.h"

enum State { BLACKS_MOVE, WHITES_MOVE, GAME_OVER };

@interface OthelloViewController
    : UIViewController <UIViewControllerRestoration> {
    othello_t board;
    enum State state;
}

@end
